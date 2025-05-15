// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "combinedpackagelist.h"
#include "combinedpackageinfo.h"

#include <QDebug>
#include <QDir>
#include <QMap>
#include <QRegularExpression>

#include "eix.pb.h"

/// Constructor just saves the package directory
CombinedPackageList::CombinedPackageList(const QString &pkgDir)
    : _pkgDirectory(pkgDir)
{
}

/*!
 * Reads the eix data and portage package database, and merges them
 * together to give a consistent picture of current package data.
 * Builds a list of any that are installed but not in the eix data,
 * called zombies here.
 */
void CombinedPackageList::load(const eix_proto::Collection &eix,
                               const QString &searchText)
{
    clear();
    readEixData(eix);
    readPortagePackageDatabase(searchText);
    identifyZombies();
}

/*!
 * Checks the zombie list to see if this category/package is a zombie
 * TODO - surely zombies should have versions?
 */
bool CombinedPackageList::isZombie(const std::string &categoryName,
                                   const std::string &packageName) const
{
    return _zombies.contains(
        CategoryPackageName(QString::fromStdString(categoryName),
                            QString::fromStdString(packageName)));
}

/*!
 * Returns the version map (list of versions) for given category/package.
 * This is empty if this is not a zombie.
 */
VersionMap CombinedPackageList::zombieVersions(const std::string &categoryName,
                                               const std::string &packageName)
{
    CategoryPackageName target(QString::fromStdString(categoryName),
                               QString::fromStdString(packageName));
    VersionMap result;

    auto lookup = _zombies.find(target);
    if (lookup != _zombies.end()) {
        return _packages[target];
    }
    return result;
}

/// Returns a list of the zombie packages
QStringList CombinedPackageList::zombieList() const
{
    QStringList result;
    foreach (auto pair, _zombies.values()) {
        result.append(pair.first + "/" + pair.second);
    }
    return result;
}

/// Empties the lists of packages and zombies
void CombinedPackageList::clear()
{
    _packages.clear();
    _zombies.clear();
}

/*!
 * Makes a list of installed packages according to the eix output
 *
 * Reads the eix database, looks for installed packages and adds them to the
 * list. The eix database gets updated when the application starts and whenever
 * search filter is changed
 */
void CombinedPackageList::readEixData(const eix_proto::Collection &eix)
{
    for (int catNumber = 0; catNumber < eix.category_size(); ++catNumber) {
        const auto &cat = eix.category(catNumber);

        QString categoryName = QString::fromStdString(cat.category());

        for (int pkgNumber = 0; pkgNumber < cat.package_size(); ++pkgNumber) {
            const auto &pkg = cat.package(pkgNumber);

            QString packageName = QString::fromStdString(pkg.name());

            // qDebug() << ">>>" << categoryName + "/" + packageName << "<<<";

            for (int verNumber = 0; verNumber < pkg.version_size();
                 ++verNumber) {
                const auto &ver = pkg.version(verNumber);

                if (ver.has_installed()) {
                    addVersion(categoryName,
                               packageName,
                               QString::fromStdString(ver.id()),
                               DataOrigin::EixData);
                }

                // See what we have
                /*
                qDebug() << "   Version : " << ver.id();
                qDebug() << "   Keywords: " << ver.keywords();
                if (ver.has_keywords_effective()) {
                    qDebug() << "   Effective Keywords: "
                             << ver.keywords_effective().value();
                }
                if (ver.has_local_mask_flags()) {
                    qDebug()
                        << "   Local Mask Flags: "
                        << EixProtoHelper::toQString(ver.local_mask_flags());
                }
                if (ver.has_system_mask_flags()) {
                    qDebug()
                        << "   System Mask Flags: "
                        << EixProtoHelper::toQString(ver.system_mask_flags());
                }
                if (ver.has_local_key_flags()) {
                    qDebug()
                        << "   Local Key Flags: "
                        << EixProtoHelper::toQString(ver.local_key_flags());
                }
                if (ver.has_system_key_flags()) {
                    qDebug()
                        << "   System Key Flags: "
                        << EixProtoHelper::toQString(ver.system_key_flags());
                }
                qDebug() << "   Depend" << ver.depend();
                */
            }
        }
    }
}

/*!
 * This should be called whenever the EIX data has been read in.
 * It scans the package list for all the installed packages, and merges them
 * into the packages list.
 *
 * If there are search filters, just update existing entries.
 *
 * TODO - the filter thing is going to cause problems with zombies
 *      - if a cat/pack has been removed from portage, it's zombie will be
 *        ignored.
 */
void CombinedPackageList::readPortagePackageDatabase(const QString &searchText)
{
    foreach (const auto categoryInfo,
             _pkgDirectory.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs)) {

        // e.g. /var/db/pkg/dev-qt

        QDir categoryDir = categoryInfo.absoluteFilePath();

        QString categoryName = categoryInfo.fileName();

        foreach (const auto packageInfo,
                 categoryDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs)) {

            // Now extract just the 'base filename' of this directory from the
            // path and then split this into package name and version. The name
            // has one or more hyphens, dividing it into fields. The version is
            // from the first field starting with a digit. So it should be easy,
            // right?
            // e.g. "qt-creator-12.4.3" -> ("qt-creator", "12.4.3")

            QString filename = packageInfo.fileName();

            int posver = filename.indexOf(QRegularExpression("\\-\\d"));
            if (posver < 0) {
                qCritical()
                    << "CombinedPackageList::readPortagePackageDatabase:"
                    << filename << "**** No version found";
            } else {
                QString packageName = filename.sliced(0, posver);
                QString packageVersion = filename.sliced(posver + 1);

                // The only filter need to check is the search text,
                // the select all/installed/world don't affect pkgdb
                // items:
                //   - all : n/a, pkgdb are always installed
                //   - installed : pkgdb by definition are installed
                //   - world : pkgdb are installed and eix will set type
                bool include =
                    searchText.isEmpty() ||
                    packageName.contains(searchText, Qt::CaseInsensitive);

                if (include) {
                    addVersion(categoryName,
                               packageName,
                               packageVersion,
                               DataOrigin::PkgData,
                               packageInfo.absoluteFilePath());
                }
            }
        }
    }
}

/*!
 * Create a list of zombie packages.
 *
 * An installed package is a zombie if it is present in the pkg database
 * but not known in the eix database.
 *
 * One way this can happen if you've installed new packages but not run
 * eix-update.
 * This can be eliminated if you compare the install time with
 * the last eix-update run, i.e. fix it by running eix-update and rescanning
 * the databases.
 *
 * The other way is:
 *   - a package has been installed
 *   - it has been removed from portage repos, but not uninstalled
 *     - it's in the pkg database
 *     - it's not in the eix database (only takes account of the repos)
 *       - IF the category/package still exists in repo
 *         -- THEN eix will report the install as an unknown version
 *         -- ELSE eix won't say anything about it
 * This means it's an obsolete package (version) - this should
 * probably be uninstalled.
 */
void CombinedPackageList::identifyZombies()
{
    _zombies.clear();

    foreach (auto &packageKey, _packages.keys()) {
        auto versionList = _packages[packageKey];

        foreach (auto versionName, versionList.keys()) {
            auto versionItem = versionList[versionName];
            if (versionItem.inPkgDb() && !versionItem.inEixDb()) {
                _zombies.insert(packageKey);
            }
        }
    }
}

/*!
 * Adds a category/package/version to the version list. May also merge
 * into an existing entry.
 */
void CombinedPackageList::addVersion(const QString &categoryName,
                                     const QString &packageName,
                                     const QString &versionName,
                                     CombinedPackageList::DataOrigin origin,
                                     const QString &packagePath)
{
    auto key = QPair<QString, QString>(categoryName, packageName);

    auto catpkg = _packages.find(key);
    if (catpkg == _packages.end()) {
        catpkg = _packages.insert(key, VersionMap());
    }
    auto &versionList = catpkg.value();

    auto versionItem = versionList.find(versionName);
    if (versionItem == versionList.end()) {
        versionItem =
            versionList.insert(versionName,
                               CombinedPackageInfo(versionName, packagePath));
    } else if (!packagePath.isEmpty()) {
        versionItem.value().setVersionDir(QDir(packagePath));
    }
    switch (origin) {
    case DataOrigin::EixData:
        versionItem.value().setEixDb(true);
        break;
    case DataOrigin::PkgData:
        versionItem.value().setPkgDb(true);
        break;
    }
}
