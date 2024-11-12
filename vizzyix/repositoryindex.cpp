// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "repositoryindex.h"
#include "applicationdata.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QtLogging>

RepositoryIndex::RepositoryIndex()
{
}

/*!
 * Loads the list of repositories from /etc/portage/repos.conf (a
 * file or directory of files.
 */
bool RepositoryIndex::load()
{
    repoMap_.clear();

    QFileInfo info(ApplicationData::reposConfFile);
    if (!info.exists()) {
        // This should not fail on a gentoo system.
        qCritical() << "Missing " << info.absoluteFilePath();
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
        qCritical() << info.absoluteFilePath() << "is not a file or directory!";
        return false;
    }

    if (confFiles.length() == 0) {
        // There should be at least one conf file present.
        qCritical() << "No repositories found!";
        return false;
    }

    // The repos.conf files are ini files, but not quite compatible with the
    // QSettings format. It's easy to parse ini files with re's, so just DIY.
    QRegularExpression commentLine("^#");
    QRegularExpression repoLine("^\\[([\\w\\-]+)\\]$"); // [some-thing]
    QRegularExpression nameVarLine(
        "^([\\w\\-]+)\\s*=\\s*(.*)$"); // some-thing = else

    foreach (const auto &confFile, confFiles) {
        QFile inputFile(confFile);

        if (inputFile.open(QIODevice::ReadOnly)) {
            QString currentSection;
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
                    // e.g. [gentoo]

                    currentSection = repoMatch.captured(1);
                    continue;
                }

                auto nameVarMatch = nameVarLine.match(line);
                if (nameVarMatch.hasMatch()) {
                    QString name(nameVarMatch.captured(1));
                    QString value(nameVarMatch.captured(2));

                    if (name == "location") {
                        // e.g. location = /var/db/repos/gentoo
                        repoMap_.insert(currentSection, value);
                    } else if (name == "main-repo" &&
                               currentSection == "DEFAULT") {
                        // e.g. main-repo = gentoo
                        mainRepo_ = value;
                    }
                } else {
                    qWarning() << confFile << "(unknown format) : " << line;
                    repoMap_.clear();
                    return false;
                }
            }
            inputFile.close();
        }
    }
    return true;
}

/*!
 * Returns the name of the default repository
 */
QString RepositoryIndex::mainRepo() const
{
    return mainRepo_;
}

/*!
 * Returns a list of the repository names (unsorted)
 */
QStringList RepositoryIndex::repoList() const
{
    return repoMap_.keys();
}

/*!
 * Looks for the repository name in the list, and returns
 * the absolute path to the repository - blank if the
 * repositotory is not known.
 *
 * A blank repository name gets the path of the default repository.
 */
QString RepositoryIndex::find(const QString &repoName) const
{
    auto match = repoMap_.find(repoName.isEmpty() ? mainRepo() : repoName);
    if (match != repoMap_.end()) {
        return match.value();
    }
    return QString();
}

// TODO Remove following if can't find another use for it
/* =================================================================
 /--*!
 * Reads the text file and matches it's content against the
 * regex.
 *
 * Returns the regex match object.
 *--/
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
===================================================================*/
