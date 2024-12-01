// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "packagereportitem.h"
#include "combinedpackagelist.h"
#include "eixprotohelper.h"

#include <QBrush>
#include <QDebug>
#include <QIcon>
#include <string>
#include <utility>

#include "eix.pb.h"

const QVector<Qt::ItemDataRole> PackageReportItem::oRole({Qt::DecorationRole,
                                                          Qt::DisplayRole,
                                                          Qt::DisplayRole,
                                                          Qt::DisplayRole,
                                                          Qt::DisplayRole});

QVariant PackageReportItem::oBoldFont;

int PackageReportItem::columnCount()
{
    return oRole.length();
}

QVariant &PackageReportItem::boldFont()
{
    return oBoldFont;
}

void PackageReportItem::setBoldFont(QFont font)
{
    oBoldFont = QVariant(font);
}

PackageReportItem::PackageReportItem(const std::string &catName,
                                     const eix_proto::Package &pkg,
                                     VersionMap &zombies)
    : oPackageDetails(&pkg), oCatName(catName), oZombieVersions(zombies)
{
    cacheValues();
}

PackageReportItem::PackageReportItem(const PackageReportItem &item)
    : oPackageDetails(item.oPackageDetails), oCatName(item.oCatName),
      oInstallType(item.oInstallType), oIsInstalled(item.oIsInstalled),
      oVersions(item.oVersions), oZombieVersions(item.oZombieVersions)
{
}

PackageReportItem &PackageReportItem::operator=(PackageReportItem other)
{
    swap(*this, other);
    return *this;
}

void swap(PackageReportItem &first, PackageReportItem &second)
{
    using std::swap;

    swap(first.oPackageDetails, second.oPackageDetails);
    swap(first.oCatName, second.oCatName);
    swap(first.oInstallType, second.oInstallType);
    swap(first.oIsInstalled, second.oIsInstalled);
    first.oVersions.swap(second.oVersions);
    first.oZombieVersions.swap(second.oZombieVersions);
}

QVariant PackageReportItem::data(int colNumber, int role) const
{
    if (colNumber < 0 || colNumber >= columnCount())
        return QVariant();

    if (role == Qt::FontRole && installed()) {
        return boldFont();
    }

    if (dataRole(colNumber) != role)
        return QVariant();

    switch (colNumber) {
    case Column::Installed:
        if (installed()) {
            switch (installType()) {
            case eix_proto::MaskFlags_MaskFlag_WORLD:
                return QVariant(
                    QIcon(":/pkgtable/images/installed-world-marker.png"));
            case eix_proto::MaskFlags_MaskFlag_WORLD_SETS:
                return QVariant(
                    QIcon(":/pkgtable/images/installed-world-set-marker.png"));
            case eix_proto::MaskFlags_MaskFlag_MASK_SYSTEM:
                return QVariant(
                    QIcon(":/pkgtable/images/installed-system-marker.png"));
            default:
                return QVariant(
                    QIcon(":/pkgtable/images/installed-marker.png"));
            }
        } else if (installType() != eix_proto::MaskFlags_MaskFlag_UNKNOWN) {
            // I don't think this can happen, but I want to see if it does.
            return QVariant(QIcon(":/pkgtable/images/world-only-marker.png"));

        } else {
            return QVariant();
        }

    case Column::Name:
        return QVariant::fromValue(
            QString::fromStdString(packageDetails().name()));

    case Column::InstalledVersion:
        return QVariant::fromValue(oVersions.join(", "));

    case Column::AvailableVersion:
        return QVariant::fromValue(highestVersionName());

    case Column::Description:
        return QVariant::fromValue(
            QString::fromStdString(packageDetails().description()));

    default:
        return QVariant();
    }
}

Qt::ItemDataRole PackageReportItem::dataRole(int colNumber) const
{
    return oRole[colNumber];
}

std::string PackageReportItem::category() const
{
    return oCatName;
}

std::string PackageReportItem::name() const
{
    return packageDetails().name();
}

std::string PackageReportItem::description() const
{
    return packageDetails().description();
}

eix_proto::MaskFlags_MaskFlag PackageReportItem::installType() const
{
    return oInstallType;
}

bool PackageReportItem::installed() const
{
    return oIsInstalled;
}

QStringList PackageReportItem::versionNames() const
{
    if (!oZombieVersions.empty()) {
        QStringList result(oVersions);
        foreach (QString ver, oZombieVersions.keys()) {
            result.append(ver + "**");
        }
        return result;
    }
    return oVersions;
}

QString PackageReportItem::highestVersionName() const
{
    QString foundVersion("");
    auto versionIndex = packageDetails().version_size() - 1;
    auto lowestLiveIndex = -1;
    while (versionIndex >= 0) {
        const auto &ver = packageDetails().version(versionIndex);
        // Live builds start with 3 or more nines. Not really interested in
        // these except as a last resort if there is nothing else.
        if (ver.id().rfind("999", 0) == std::string::npos) {
            foundVersion = QString::fromStdString(ver.id());
            if (!EixProtoHelper::isStable(ver)) {
                foundVersion = "~" + foundVersion;
            }
            // For checking the tests
            // foundVersion = QString("%1-(%2/%3)")
            //                   .arg(foundVersion)
            //                   .arg(versionIndex)
            //                   .arg(packageDetails().version_size());
            break;
        } else {
            lowestLiveIndex = versionIndex;
        }
        versionIndex--;
    }

    if (foundVersion.isEmpty() && lowestLiveIndex >= 0) {
        const auto &ver = packageDetails().version(lowestLiveIndex);
        foundVersion = QString::fromStdString(ver.id());
        if (!EixProtoHelper::isStable(ver)) {
            foundVersion = "~" + foundVersion;
        }
    }

    return foundVersion;
}

// Some values are needed for redraws but are not straightforward to
// work out. Determine these upfront (in the constructor) and cache
// them.
void PackageReportItem::cacheValues()
{
    auto resultInstalled = false;
    auto resultInstallType = eix_proto::MaskFlags_MaskFlag_UNKNOWN;

    for (int vn = 0; vn < packageDetails().version_size(); ++vn) {
        bool versionInstalled = false;
        const eix_proto::Version &ver = packageDetails().version(vn);

        // Installed means the software is installed. If so, "world" means
        // the software was pulled in from the world file or world set,
        // otherwise it's a dependency. I think.
        if (ver.has_installed()) {
            resultInstalled = true;
            versionInstalled = true;

            resultInstallType = EixProtoHelper::classifyInstallType(ver);
        }

        if (versionInstalled) {
            // TODO : could remember the version indexes, handy later
            QString outVersion(QString::fromStdString(ver.id()));
            if (!EixProtoHelper::isStable(ver)) {
                outVersion = QString("(~)%1").arg(outVersion);
            }
            oVersions.append(outVersion);
        }
    }

    oIsInstalled = resultInstalled;   // i.e. are any versions installed
    oInstallType = resultInstallType; // for the 'highest' version installed
                                      // (TODO: return a list?)
}

const eix_proto::Package &PackageReportItem::packageDetails() const
{
    return *oPackageDetails;
}
