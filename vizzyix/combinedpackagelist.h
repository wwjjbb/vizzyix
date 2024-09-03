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
#include "localexceptions.h"

typedef QPair<QString, QString> CategoryPackageName;
typedef QMap<QString, CombinedPackageInfo> VersionMap;

enum MergeMode { MergeOnly, MergeAdd };

class CombinedPackageList
{
  public:
    CombinedPackageList(const QString &pkgDir = "var/db/pkg");

    void clear();
    void readPortagePackageDatabase(bool filtered);
    void readEixData(const eix_proto::Collection &eix);
    void identifyZombies();

    bool isZombie(const std::string &categoryName,
                  const std::string &packageName) const;
    VersionMap zombies(const std::string &categoryName,
                       const std::string &packageName);
    QStringList zombies() const;

  private:
    // No copy/swap/assign - not required for anything
    CombinedPackageList(const CombinedPackageList &);
    CombinedPackageList &operator=(CombinedPackageList &);

  private:
    enum DataOrigin { EixData, PkgData };

    void clearPackageDataFlags();
    void purgeOrphanPackages();
    void addVersion(const QString &categoryName, const QString &packageName,
                    const QString &versionName, DataOrigin origin,
                    MergeMode mode, const QString &packagePath = QString());

  private:
    const QDir pkgDirectory_;

    /*!
     * \brief packages_
     * The first string is the category, e.g. "dev-qt".
     * The second string is the package name, e.g. "qt-creator".
     * The value is a map of installed versions of the package
     */
    QMap<CategoryPackageName, VersionMap> packages_;

    QSet<CategoryPackageName> zombies_;
};
