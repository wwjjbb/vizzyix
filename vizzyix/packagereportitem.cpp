// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "packagereportitem.h"
#include "combinedpackagelist.h"
#include "eixprotohelper.h"

#include <QBrush>
#include <QIcon>
#include <utility>

#include "eix.pb.h"

const QVector<Qt::ItemDataRole> PackageReportItem::role_(
    {Qt::DecorationRole, Qt::DisplayRole, Qt::DisplayRole, Qt::DisplayRole,
     Qt::DisplayRole});

QVariant PackageReportItem::boldFont_;

int PackageReportItem::columnCount()
{
    return role_.length();
}

QVariant &PackageReportItem::boldFont()
{
    return boldFont_;
}

void PackageReportItem::setBoldFont(QFont font)
{
    boldFont_ = QVariant(font);
}

PackageReportItem::PackageReportItem(const std::string &catName,
                                     const eix_proto::Package &pkg,
                                     VersionMap &zombies)
    : packageDetails_(&pkg), catName_(catName), zombieVersions_(zombies)
{
    cacheValues();
}

PackageReportItem::PackageReportItem(const PackageReportItem &item)
    : packageDetails_(item.packageDetails_), catName_(item.catName_),
      installType_(item.installType_), installed_(item.installed_),
      versions_(item.versions_), zombieVersions_(item.zombieVersions_)
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

    swap(first.packageDetails_, second.packageDetails_);
    swap(first.catName_, second.catName_);
    swap(first.installType_, second.installType_);
    swap(first.installed_, second.installed_);
    first.versions_.swap(second.versions_);
    first.zombieVersions_.swap(second.zombieVersions_);
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
        return QVariant::fromValue(versions_.join(", "));

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
    return role_[colNumber];
}

std::string PackageReportItem::category() const
{
    return catName_;
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
    return installType_;
}

bool PackageReportItem::installed() const
{
    return installed_;
}

QStringList PackageReportItem::versionNames() const
{
    if (!zombieVersions_.empty()) {
        QStringList result(versions_);
        foreach (QString ver, zombieVersions_.keys()) {
            result.append(ver + "**");
        }
        return result;
    }
    return versions_;
}

QString PackageReportItem::highestVersionName() const
{
    QString foundVersion("");
    auto versionIndex = packageDetails().version_size() - 1;
    while (versionIndex >= 0) {
        const auto &ver = packageDetails().version(versionIndex);
        if (ver.id() != "9999") {
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
        }
        versionIndex--;
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
            versions_.append(outVersion);
        }
    }

    installed_ = resultInstalled;     // i.e. are any versions installed
    installType_ = resultInstallType; // for the 'highest' version installed
                                      // (TODO: return a list?)
}

const eix_proto::Package &PackageReportItem::packageDetails() const
{
    return *packageDetails_;
}
