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

const QVector<Qt::ItemDataRole> PackageReportItem::_role({Qt::DecorationRole,
                                                          Qt::DisplayRole,
                                                          Qt::DisplayRole,
                                                          Qt::DisplayRole,
                                                          Qt::DisplayRole});

QVariant PackageReportItem::_boldFont;

int PackageReportItem::columnCount()
{
    return _role.length();
}

QVariant &PackageReportItem::boldFont()
{
    return _boldFont;
}

void PackageReportItem::setBoldFont(QFont font)
{
    _boldFont = QVariant(font);
}

PackageReportItem::PackageReportItem(const std::string &catName,
                                     const eix_proto::Package &pkg,
                                     VersionMap &zombies)
    : _packageDetails(&pkg), _catName(catName), _zombieVersions(zombies)
{
    cacheValues();
}

PackageReportItem::PackageReportItem(const PackageReportItem &item)
    : _packageDetails(item._packageDetails), _catName(item._catName),
      _installType(item._installType), _isInstalled(item._isInstalled),
      _versions(item._versions), _zombieVersions(item._zombieVersions)
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

    swap(first._packageDetails, second._packageDetails);
    swap(first._catName, second._catName);
    swap(first._installType, second._installType);
    swap(first._isInstalled, second._isInstalled);
    first._versions.swap(second._versions);
    first._zombieVersions.swap(second._zombieVersions);
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
        return QVariant::fromValue(_versions.join(", "));

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
    return _role[colNumber];
}

std::string PackageReportItem::category() const
{
    return _catName;
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
    return _installType;
}

bool PackageReportItem::installed() const
{
    return _isInstalled;
}

QStringList PackageReportItem::versionNames() const
{
    if (!_zombieVersions.empty()) {
        QStringList result(_versions);
        foreach (QString ver, _zombieVersions.keys()) {
            result.append(ver + "**");
        }
        return result;
    }
    return _versions;
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
            // foundVersion = QStringLiteral("%1-(%2/%3)")
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
                outVersion = QStringLiteral("(~)%1").arg(outVersion);
            }
            _versions.append(outVersion);
        }
    }

    _isInstalled = resultInstalled;   // i.e. are any versions installed
    _installType = resultInstallType; // for the 'highest' version installed
                                      // (TODO: return a list?)
}

const eix_proto::Package &PackageReportItem::packageDetails() const
{
    return *_packageDetails;
}
