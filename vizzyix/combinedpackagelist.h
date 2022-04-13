// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#ifndef COMBINEDPACKAGELIST_H
#define COMBINEDPACKAGELIST_H

#include <QMap>
#include <QSet>

#include "combinedpackageinfo.h"
#include "eix.pb.h"

typedef QPair<QString, QString> CategoryPackageName;
typedef QMap<QString, CombinedPackageInfo> VersionMap;

class CombinedPackageList
{
  public:
    CombinedPackageList();

    void readPackageData();
    void readEixData(const eix_proto::Collection &eix);
    bool isZombie(const std::string &categoryName,
                  const std::string &packageName) const;
    VersionMap zombies(const std::string &categoryName,
                       const std::string &packageName);
    QStringList zombies() const;
    void identifyZombies();

  private:
    // No copy/swap/assign - not required for anything
    CombinedPackageList(const CombinedPackageList &other);
    friend void swap(CombinedPackageList &first, CombinedPackageList &second);
    CombinedPackageList &operator=(CombinedPackageList &other);

  private:
    enum DataOrigin { EixData, PkgData };

    void clearPackageDataFlags();
    void purgeOrphanPackages();
    void addVersion(const QString &categoryName, const QString &packageName,
                    const QString &versionName, DataOrigin origin,
                    const QString &packagePath = QString());

  private:
    /*!
     * \brief packages_
     * The first string is the category, e.g. "dev-qt".
     * The second string is the package name, e.g. "qt-creator".
     * The value is a map of installed versions of the package
     */
    QMap<CategoryPackageName, VersionMap> packages_;

    QSet<CategoryPackageName> zombies_;

    bool eixDataPresent = false;
};

#endif // COMBINEDPACKAGELIST_H
