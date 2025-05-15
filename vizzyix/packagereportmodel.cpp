// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "packagereportmodel.h"
#include "combinedpackagelist.h"

#include <iostream>

PackageReportModel::PackageReportModel(QObject *)
{
}

QVariant PackageReportModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

    if (idx.row() < rowCount() && idx.column() < columnCount()) {
        return _packages[idx.row()].data(idx.column(), role);
    }

    return QVariant();
}

QVariant PackageReportModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case PackageReportItem::Column::Installed:
                return QVariant();
                break;

            case PackageReportItem::Column::Name:
                return QVariant("Package");
                break;

            case PackageReportItem::Column::InstalledVersion:
                return QVariant("Version");
                break;

            case PackageReportItem::Column::AvailableVersion:
                return QVariant("Available");
                break;

            case PackageReportItem::Column::Description:
                return QVariant("Description");
                break;

            default:
                break;
            }
        } else if (role == Qt::InitialSortOrderRole) {
            if (section == PackageReportItem::Column::Name) {
                return QVariant::fromValue(Qt::AscendingOrder);
            }
        }
    }
    return QVariant();
}

int PackageReportModel::rowCount(const QModelIndex &idx) const
{
    return idx.isValid() ? 0 : _packages.size();
}

int PackageReportModel::columnCount(const QModelIndex &idx) const
{
    return idx.isValid() ? 0 : PackageReportItem::columnCount();
}

void PackageReportModel::startUpdate()
{
    beginResetModel();
}

void PackageReportModel::endUpdate()
{
    endResetModel();
}

void PackageReportModel::addPackage(const std::string &catName,
                                    const eix_proto::Package &package,
                                    VersionMap &zombies)
{
    _packages.append(PackageReportItem(catName, package, zombies));
}

void PackageReportModel::clear()
{
    _packages.clear();
}

const PackageReportItem &PackageReportModel::packageItem(int n)
{
    return _packages[n];
}
