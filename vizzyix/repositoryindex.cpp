// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "repositoryindex.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>

const QString RepositoryIndex::reposConf = "/etc/portage/repos.conf";

RepositoryIndex::RepositoryIndex()
{
}

bool RepositoryIndex::load()
{
    repoMap_.clear();

    QFileInfo info(reposConf);
    if (!info.exists()) {
        // This should not fail on a gentoo system.
        // TODO: really ought to log something, or throw exception
        return false;
    }

    QStringList confFiles;
    if (info.isFile()) {
        confFiles.append(info.absoluteFilePath());
    } else if (info.isDir()) {
        QDir confDir(info.absoluteFilePath());
        foreach (const auto confFile,
                 confDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files)) {
            confFiles.append(confFile.absoluteFilePath());
        }

    } else {
        // If it's not a file or directory I don't know what it is.
        // (the tests treat symlinks as the target, so its not a
        // symlink issue).
        // TODO: really ought to log something, or throw exception.
        return false;
    }

    if (confFiles.length() == 0) {
        // There should be at least one conf file present.
        // TODO: log or throw exception
        return false;
    }

    // The repos.conf files are ini files, but not quite compatible with the
    // QSettings format. It's easy to parse ini files with re's, so just DIY.
    QRegularExpression commentLine("^#");
    QRegularExpression repoLine("^\\[([\\w\\-]+)\\]$");
    QRegularExpression nameVarLine("^([\\w\\-]+)\\s*=\\s*(.*)$");

    foreach (const auto confFile, confFiles) {
        QString currentSection;

        QFile inputFile(confFile);
        if (inputFile.open(QIODevice::ReadOnly)) {
            QTextStream inputStream(&inputFile);
            while (!inputStream.atEnd()) {
                QString line = inputStream.readLine().trimmed();
                if (line.isEmpty()) {
                    // Ignore empty lines
                    continue;
                }

                auto commentMatch = commentLine.match(line);
                if (commentMatch.hasMatch()) {
                    // Ignore comment lines
                    continue;
                }

                auto repoMatch = repoLine.match(line);
                if (repoMatch.hasMatch()) {
                    // qDebug() << "Section:" << repoMatch.captured(1);

                    currentSection = repoMatch.captured(1);
                    continue;
                }

                auto nameVarMatch = nameVarLine.match(line);
                if (nameVarMatch.hasMatch()) {
                    QString name(nameVarMatch.captured(1));
                    QString value(nameVarMatch.captured(2));

                    // qDebug() << "Name:" << name << "Value:" << value;

                    if (name == "location") {
                        repoMap_.insert(currentSection, value);
                    } else if (name == "main-repo" &&
                               currentSection == "DEFAULT") {
                        mainRepo_ = value;
                    }
                } else {
                    // TODO: report this - it's far more likely than the other
                    // problems above because this all depends on user edited
                    // file content.
                    qDebug()
                        << "Unknown line format: " << confFile << ":" << line;

                    repoMap_.clear();
                    return false;
                }
            }
            inputFile.close();
        }
    }
    checkMakeConf();
    return true;
}

QString RepositoryIndex::mainRepo() const
{
    return mainRepo_;
}

QStringList RepositoryIndex::repoList() const
{
    return repoMap_.keys();
}

// An empty repo name is treated as the main repo
QString RepositoryIndex::find(const QString &repo) const
{
    auto match = repoMap_.find(repo.isEmpty() ? mainRepo() : repo);
    if (match != repoMap_.end()) {
        return match.value();
    }
    return QString();
}

QRegularExpressionMatch scanFile(QFile &file, QString regex)
{
    QRegularExpressionMatch result;

    if (file.open(QIODevice::ReadOnly)) {
        QTextStream inputStream(&file);
        auto data = inputStream.readAll();

        result = QRegularExpression(regex).match(data);
        file.close();
    }

    return result;
}

/*!
 * \fn RepositoryIndex::checkMakeConf
 * Search make.conf for PORTDIR_OVERLAY looking for a line like this:
 *   PORTDIR_OVERLAY="/usr/local/portage ${PORTDIR_OVERLAY}"
 *
 * /usr/local/portage/metadata/layout.conf is an ini file without
 * a section, and the only interesting name/value is:
 *    repo-name = fred
 *
 * Fails silently if PORTDIR_OVERLAY can't be found, or if it can't get
 * the repo-name from the repository layout.conf.
 */
void RepositoryIndex::checkMakeConf()
{
    QFile makeConfFile("/etc/portage/make.conf");

    auto checkPortdir =
        scanFile(makeConfFile, "\\vPORTDIR_OVERLAY=\"([^ ]+) \\$");
    if (checkPortdir.hasMatch()) {
        QString portdirOverlay = checkPortdir.captured(1);

        QFile layoutConfFile(
            QString("%1/metadata/layout.conf").arg(portdirOverlay));
        if (layoutConfFile.exists()) {
            auto checkLayout =
                scanFile(layoutConfFile, "\\vrepo-name\\s*=\\s*(.+)\\v");
            if (checkLayout.hasMatch()) {
                QString repoName = checkLayout.captured(1);
                repoMap_.insert(repoName, portdirOverlay);
            }
        }
    }
}
