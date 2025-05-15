// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QFont>
#include <QStringList>
#include <QVariant>

#include "combinedpackagelist.h"
#include "eix.pb.h"

class PackageReportItem
{
  public:
    explicit PackageReportItem(const std::string &catName,
                               const eix_proto::Package &pkg,
                               VersionMap &zombies);

    PackageReportItem(const PackageReportItem &item);
    friend void swap(PackageReportItem &first, PackageReportItem &second);
    PackageReportItem &operator=(PackageReportItem other);

    static int columnCount();
    static QVariant &boldFont();
    static void setBoldFont(QFont font);

    QVariant data(int colNumber, int dataRole) const;
    Qt::ItemDataRole dataRole(int colNumber) const;

    std::string category() const;
    std::string name() const;
    std::string description() const;
    eix_proto::MaskFlags_MaskFlag installType() const;
    bool installed() const;
    QStringList versionNames() const;
    QString highestVersionName() const;
    const eix_proto::Package &packageDetails() const;

    /// There's an enum value for each column in the package report
    enum Column {
        Installed,
        Name,
        InstalledVersion,
        AvailableVersion,
        Description
    };

  private:
    void cacheValues();

  private:
    /// _role[N] -> role of column N
    const static QVector<Qt::ItemDataRole> _role;

    /// Holds the QFont value for bold text
    static QVariant _boldFont;

    /// Pointer to the EIXDB data for the package
    const eix_proto::Package *_packageDetails;

    /// The catalog name
    std::string _catName;

    /// The WORLD/SET/SYSTEM/UNKNOWN indicator
    eix_proto::MaskFlags_MaskFlag _installType;

    /// Whether any versions of package are installed
    bool _isInstalled;

    /// Package versions, in ascending order
    QStringList _versions;

    /// Versions of this package that are zombies
    VersionMap _zombieVersions;
};
