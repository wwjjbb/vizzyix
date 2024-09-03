// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QAbstractItemModel>

#include "categorytreeitem.h"

class CategoryTreeModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    explicit CategoryTreeModel(QObject *parent = nullptr);
    ~CategoryTreeModel();

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void startUpdate();
    void endUpdate();
    void addCategory(const uint categoryIndex, const QString &categoryName,
                     const size_t categorySize);
    void clear();

    const CategoryTreeItem *allItem() const;

  private:
    CategoryTreeItem *rootItem_;
    CategoryTreeItem *allItem_;
};
