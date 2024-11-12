// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "categorytreemodel.h"

#include <QDebug>
#include <QtLogging>

CategoryTreeModel::CategoryTreeModel(QObject *)
{
    // The tree model always contains the root node (headers), and a child of
    // this which is the visible root of all the categories.

    rootItem_ = CategoryTreeItem::newRootItem(
        {tr("Categories"), tr("Pkgs"), tr("Idx")});

    allItem_ = rootItem_->appendChild({tr("All"), 0, -1});
}

CategoryTreeModel::~CategoryTreeModel()
{
    delete rootItem_;
}

QVariant CategoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    CategoryTreeItem *item =
        static_cast<CategoryTreeItem *>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags CategoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

QVariant CategoryTreeModel::headerData(int section,
                                       Qt::Orientation orientation,
                                       int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole &&
        section >= 0 && section < columnCount()) {
        return rootItem_->data(section);
    }

    return QVariant();
}

QModelIndex
CategoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    CategoryTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem_;
    else
        parentItem = static_cast<CategoryTreeItem *>(parent.internalPointer());

    CategoryTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);

    return QModelIndex();
}

QModelIndex CategoryTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    CategoryTreeItem *childItem =
        static_cast<CategoryTreeItem *>(index.internalPointer());
    CategoryTreeItem *parentItem = childItem->parentItem();

    if (parentItem == rootItem_)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int CategoryTreeModel::rowCount(const QModelIndex &parent) const
{
    CategoryTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem_;
    else
        parentItem = static_cast<CategoryTreeItem *>(parent.internalPointer());

    return parentItem->childCount();
}

int CategoryTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<CategoryTreeItem *>(parent.internalPointer())
            ->columnCount();

    return rootItem_->columnCount();
}

void CategoryTreeModel::startUpdate()
{
    beginResetModel();
}

void CategoryTreeModel::endUpdate()
{
    endResetModel();
}

/*!
 * \brief CategoryTreeModel::addCategory adds an eix category to the tree.
 * \param categoryIndex - entry number, 0+, in the eix collection of categories.
 * \param categoryName - the name of the category, which by convention usually
 * contains one dash \param categorySize - number of packages in the category,
 * 1+
 */
void CategoryTreeModel::addCategory(const uint categoryIndex,
                                    const QString &categoryName,
                                    const size_t categorySize)
{
    // There should be one or two parts to the name, i.e. one dash
    // Generally it's just "virtual" with one part.
    // A dash is not expected as the first character

    int dashPos = categoryName.indexOf('-');
    bool splitName = (dashPos >= 0);

    QString part1 = splitName ? categoryName.sliced(0, dashPos) : categoryName;
    QString part2 = splitName ? categoryName.sliced(dashPos + 1) : "";

    if (categoryName == "")
        qWarning() << "(addCategory) blank category name at index"
                   << categoryIndex;

    if (dashPos == 0)
        qWarning() << "(addCategory) category name starts with dash:"
                   << categoryName;

    if (part2.indexOf('-') >= 0)
        qWarning() << "(addCategory) category name contains 2+ dashes:"
                   << categoryName;

    if (!splitName) {
        QVector<QVariant> data;
        data << part1 << QVariant::fromValue(categorySize)
             << QVariant::fromValue(categoryIndex);

        (void)allItem_->appendChild(data);
    } else {
        CategoryTreeItem *top = allItem_->findChild(part1);
        if (!top) {
            QVector<QVariant> topData;
            topData << part1 << 0 << -1;

            top = allItem_->appendChild(topData);
        }

        QVector<QVariant> data;
        data << part2 << QVariant::fromValue(categorySize)
             << QVariant::fromValue(categoryIndex);

        (void)top->appendChild(data);
        top->setPackageCount(top->packageCount() + categorySize);
    }
    allItem_->setPackageCount(allItem_->packageCount() + categorySize);
}

void CategoryTreeModel::clear()
{
    allItem_->freeChildItems();
    allItem_->setData(CategoryTreeItem::Column::PkgCount, 0);
}

const CategoryTreeItem *CategoryTreeModel::allItem() const
{
    return allItem_;
}
