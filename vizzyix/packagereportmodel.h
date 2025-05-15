// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <QVector>

#include "combinedpackagelist.h"
#include "eix.pb.h"
#include "packagereportitem.h"

class PackageReportModel : public QAbstractTableModel
{
    Q_OBJECT
  public:
    explicit PackageReportModel(QObject *parent = nullptr);
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void startUpdate();
    void endUpdate();
    void addPackage(const std::string &catName,
                    const eix_proto::Package &package,
                    VersionMap &zombies);
    void clear();
    const PackageReportItem &packageItem(int n);

  private:
    QVector<PackageReportItem> _packages;
};
