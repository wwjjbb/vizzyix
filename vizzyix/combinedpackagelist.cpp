// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "combinedpackagelist.h"
#include "combinedpackageinfo.h"

#include <QDebug>
#include <QDir>
#include <QMap>

#include "eix.pb.h"

/*
 * CombinedPackageList::CombinedPackageList
 *
 * Constructor - initial placeholder
 */
CombinedPackageList::CombinedPackageList()
{
}

/*
 * CombinedPackageList::readPortagePackageDatabase
 *
 * Makes a list of all installed packages by scanning the pkg database
 */
void CombinedPackageList::readPortagePackageDatabase(
    const eix_proto::Collection &eix)
{
    clearPackageDataFlags();

    // TODO: find pkg db from a system variable, or config setting
    QDir pkgDir("/var/db/pkg");
    foreach (const auto categoryInfo,
             pkgDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs)) {

        // e.g. /var/db/pkg/dev-qt

        QDir categoryDir = categoryInfo.absoluteFilePath();

        QString categoryName = categoryInfo.fileName();

        foreach (const auto packageInfo,
                 categoryDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs)) {

            // Now need to extract just the 'base filename' of this
            // directory from the path and then split this into psckage name
            // and version. The name has one or more hyphens, splitting it
            // into fields. The version is from the first field starting
            // with a digit. So it should be easy, right?

            // e.g. qt-creator-12.4.3
            QString filename = packageInfo.fileName();

            int posver = filename.indexOf(QRegExp("\\-\\d"));
            if (posver < 0) {
                // TODO: never seen, is there any need to report it?
                qDebug() << filename << "**** No version found";
            } else {
                QStringRef packageName(&filename, 0, posver);
                QStringRef packageVersion(&filename, posver + 1,
                                          filename.length() - (posver + 1));

                addVersion(categoryName, packageName.toString(),
                           packageVersion.toString(), PkgData,
                           packageInfo.absoluteFilePath());
            }
        }
    }

    mergeEixData(eix);
    identifyZombies();
}

/*
 * CombinedPackageList::mergeEixData
 *
 * Makes a list of insalled packages according to the eix database
 *
 * Reads the eix database, looks for installed packages and adds them to the
 * list. It is expected this will only be done once, when this app starts up and
 * does the complete grab of eix data.
 *
 * eix:
 *     The eix database
 *
 */
void CombinedPackageList::mergeEixData(const eix_proto::Collection &eix)
{
    for (int catNumber = 0; catNumber < eix.category_size(); ++catNumber) {
        const auto &cat = eix.category(catNumber);

        QString categoryName = QString::fromStdString(cat.category());

        for (int pkgNumber = 0; pkgNumber < cat.package_size(); ++pkgNumber) {
            const auto &pkg = cat.package(pkgNumber);

            QString packageName = QString::fromStdString(pkg.name());

            for (int verNumber = 0; verNumber < pkg.version_size();
                 ++verNumber) {
                const auto &ver = pkg.version(verNumber);

                if (ver.has_installed()) {
                    addVersion(categoryName, packageName,
                               QString::fromStdString(ver.id()), EixData);
                }
            }
        }
    }
}

VersionMap CombinedPackageList::zombies(const std::string &categoryName,
                                        const std::string &packageName)
{
    CategoryPackageName target(QString::fromStdString(categoryName),
                               QString::fromStdString(packageName));
    VersionMap result;

    auto lookup = zombies_.find(target);
    if (lookup != zombies_.end()) {
        return packages_[target];
    }
    return result;
}

QStringList CombinedPackageList::zombies() const
{
    QStringList result;
    foreach (auto pair, zombies_.values()) {
        result.append(pair.first + "/" + pair.second);
    }
    return result;
}

bool CombinedPackageList::isZombie(const std::string &categoryName,
                                   const std::string &packageName) const
{
    return zombies_.contains(
        CategoryPackageName(QString::fromStdString(categoryName),
                            QString::fromStdString(packageName)));
}

/*
 * CombinedPackageList::clearPackageData
 *
 * Reset all pkgDb flags
 */
void CombinedPackageList::clearPackageDataFlags()
{
    foreach (auto &packageKey, packages_.keys()) {
        auto versionList = packages_[packageKey];
        foreach (auto &versionName, versionList.keys()) {
            versionList[versionName].setPkgDb(false);
        }
    }
}

/*
 * CombinedPackageList::purgeOrphanPackages
 *
 * Remove any packages not in package db and not in eix db
 */
void CombinedPackageList::purgeOrphanPackages()
{
    // This is the old problem about deleting entries from the list you're
    // scanning...
    QVector<CategoryPackageName> unwantedPackages;

    foreach (auto &packageKey, packages_.keys()) {
        auto versionList = packages_[packageKey];

        QVector<QString> unwantedVersions;

        foreach (auto versionName, versionList.keys()) {
            auto versionItem = versionList[versionName];
            if (!versionItem.inPkgDb() && !versionItem.inEixDb()) {
                unwantedVersions.append(versionName);
            }
        }

        if (!unwantedVersions.isEmpty()) {
            foreach (auto versionName, unwantedVersions) {
                auto which = versionList.find(versionName);
                if (which != versionList.end()) {
                    versionList.erase(which);
                }
            }
        }

        if (versionList.isEmpty()) {
            unwantedPackages.append(packageKey);
        }
    }

    if (!unwantedPackages.isEmpty()) {
        foreach (auto packageName, unwantedPackages) {
            auto which = packages_.find(packageName);
            if (which != packages_.end()) {
                packages_.erase(which);
            }
        }
    }
}

/*
 * CombinedPackageList::identifyZombies
 *
 * Create a list of zombie packages.
 *
 * An installed package is a zombie if it is present in the pkg database
 * but not known in the eix database.
 * One way this can happen if you've installed new packages but not run
 * eix-update. The other way is:
 *   - a package has been installed
 *   - it has been removed from portage repos, but not uninstalled
 *     - it's in the pkg database
 *     - it's not in the eix database (only takes account of the repos)
 *       - IF the category/package still exists in repo
 *         -- THEN eix will report the install as an unknown version
 *         -- ELSE eix won't say anything about it
 * The first case can be eliminated if you compare the install time with
 * the last eix-update run, i.e. fix it by running eix-update and rescanning
 * the databases.
 * The second case means it's an obsolete pacjkage (version) - this should
 * probably be uninstalled.
 */
void CombinedPackageList::identifyZombies()
{
    zombies_.clear();

    foreach (auto &packageKey, packages_.keys()) {
        auto versionList = packages_[packageKey];

        foreach (auto versionName, versionList.keys()) {
            auto versionItem = versionList[versionName];
            if (versionItem.inPkgDb() && !versionItem.inEixDb()) {
                zombies_.insert(packageKey);
            }
        }
    }
}

void CombinedPackageList::addVersion(const QString &categoryName,
                                     const QString &packageName,
                                     const QString &versionName,
                                     CombinedPackageList::DataOrigin origin,
                                     const QString &packagePath)
{
    auto key = QPair<QString, QString>(categoryName, packageName);

    auto catpkg = packages_.find(key);
    if (catpkg == packages_.end()) {
        catpkg = packages_.insert(key, QMap<QString, CombinedPackageInfo>());
    }
    auto &versionList = catpkg.value();

    auto versionItem = versionList.find(versionName);
    if (versionItem == versionList.end()) {
        versionItem = versionList.insert(
            versionName, CombinedPackageInfo(versionName, packagePath));
    } else if (!packagePath.isEmpty()) {
        versionItem.value().setVersionDir(QDir(packagePath));
    }
    switch (origin) {
    case EixData:
        versionItem.value().setEixDb(true);
        break;
    case PkgData:
        versionItem.value().setPkgDb(true);
        break;
    }
}
