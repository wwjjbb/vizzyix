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
    if (!_repositoryMap.empty()) {
        // It's already loaded the Only need to do this once
        return true;
    }

    bool ok{true};

    QFileInfo info(ApplicationData::reposConfFile);
    if (!info.exists()) {
        // This should not fail on a gentoo system.
        qCritical() << "* Missing repo configuration \""
                    << info.absoluteFilePath() << "\"";
        ok = false;
    }

    QStringList confFiles;

    if (ok) {
        if (info.isFile()) {
            confFiles.append(info.absoluteFilePath());
        } else if (info.isDir()) {
            QDir confDir(info.absoluteFilePath());
            foreach (
                const auto confFile,
                confDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files)) {
                confFiles.append(confFile.absoluteFilePath());
            }

        } else {
            // If it's not a file or directory I don't know what it is.
            // (the tests treat symlinks as the target, so its not a
            // symlink issue).
            qCritical() << info.absoluteFilePath()
                        << "is not a file or directory!";
            ok = false;
        }
    }

    if (ok) {
        if (confFiles.length() == 0) {
            // There should be at least one conf file present.
            qCritical() << "No repositories found!";
            ok = false;
        }
    }

    if (ok) {
        // The repos.conf files are ini files, but not quite compatible with the
        // QSettings format. It's easy to parse ini files with re's, so just
        // DIY.
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
                            _repositoryMap.insert(currentSection, value);
                        } else if (name == "main-repo" &&
                                   currentSection == "DEFAULT") {
                            // e.g. main-repo = gentoo
                            _mainRepository = value;
                        }
                    } else {
                        // Something other than comment/blank, section or
                        // name-value has been found. This application needs an
                        // update or the file really does contain dud data.
                        qWarning()
                            << "* \"" << confFile
                            << "\" unknown format here: \"" << line << "\"";
                        ok = false;
                        break;
                    }
                }
                inputFile.close();
            } else {
                qWarning() << "* \"" << confFile << "\" could not be opened";
                ok = false;
            }
        }
    }

    // Final checks
    if (_mainRepository.isEmpty()) {
        qWarning() << "* No repository is defined as the \"main-repo\"";
        ok = false;
    }

    if (_repositoryMap.empty()) {
        qWarning() << "* No repositories are defined!";
        ok = false;
    }

    // Clear all repo data on error
    if (!ok) {
        qWarning() << "Problems loading repository information from \""
                   << ApplicationData::reposConfFile
                   << "\" (see messages above)";
        _mainRepository.clear();
        _repositoryMap.clear();
    }

    return ok;
}

/*!
 * Looks for the repository name in the list, and returns
 * the absolute path to the repository - blank if the
 * repositotory is not known.
 *
 * A blank repository name gets the path of the default repository.
 */
QString RepositoryIndex::find(const QString &name) const
{
    auto match = _repositoryMap.find(name.isEmpty() ? _mainRepository : name);
    if (match != _repositoryMap.end()) {
        return match.value();
    }
    return QString();
}
