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

    enum Column {
        Installed,
        Name,
        InstalledVersion,
        AvailableVersion,
        Description
    };

  private:
    void cacheValues();

    // Need to define static attributes in the cpp file
    const static QVector<Qt::ItemDataRole> role_;
    static QVariant boldFont_;

    const eix_proto::Package *packageDetails_;
    std::string catName_;
    eix_proto::MaskFlags_MaskFlag installType_;
    bool installed_;
    QStringList versions_;
    VersionMap zombieVersions_;
};
