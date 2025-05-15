// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QDir>
#include <QMap>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>

#include "combinedpackageinfo.h"
#include "eix.pb.h"

typedef QPair<QString, QString> CategoryPackageName;
typedef QMap<QString, CombinedPackageInfo> VersionMap;

class CombinedPackageList
{
  public:
    CombinedPackageList(const QString &pkgDir);

    CombinedPackageList(const CombinedPackageList &) = delete;
    CombinedPackageList &operator=(CombinedPackageList &) = delete;

    void load(const eix_proto::Collection &eix, const QString &searchText);

    bool isZombie(const std::string &categoryName,
                  const std::string &packageName) const;
    VersionMap zombieVersions(const std::string &categoryName,
                              const std::string &packageName);
    QStringList zombieList() const;

  private:
    /// Type to identify where this entry comes from
    enum DataOrigin { EixData, PkgData };

    void clear();
    void readEixData(const eix_proto::Collection &eix);
    void readPortagePackageDatabase(const QString &searchText);
    void identifyZombies();

    void addVersion(const QString &categoryName,
                    const QString &packageName,
                    const QString &versionName,
                    DataOrigin origin,
                    const QString &packagePath = QString());

  private:
    /// The root directory of the package database, i.e. /var/db/pkg
    const QDir _pkgDirectory;

    /*!
     * The key is the category + package name, e.g. "dev-qt" + "qt-creator".
     * The value is a map of installed versions of the package
     */
    QMap<CategoryPackageName, VersionMap> _packages;

    /*!
     * Category and package name of each zombie.
     * TODO - surely zombie's have version numbers!
     */
    QSet<CategoryPackageName> _zombies;
};
