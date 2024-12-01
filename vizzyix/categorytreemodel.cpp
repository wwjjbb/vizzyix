// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "categorytreemodel.h"

#include <QDebug>
#include <QtLogging>

/*!
 * Creates the column titles and a top level node called "All".
 */
CategoryTreeModel::CategoryTreeModel(QObject *)
{
    // The tree model always contains the root node (headers), and a child of
    // this which is the visible root of all the categories.

    oRootItem = CategoryTreeItem::newRootItem(
        {tr("Categories"), tr("Pkgs"), tr("Idx")});

    oAllItem = oRootItem->appendChild({tr("All"), 0, -1});
}

/*!
 * Destructor frees up all child nodes
 */
CategoryTreeModel::~CategoryTreeModel()
{
    delete oRootItem;
}

/*!
 * Given a model index and role, this returns the data for the associated
 * column. It only responds to DisplayRole, returning blanks otherwise.
 */
QVariant CategoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    const CategoryTreeItem *item =
        static_cast<CategoryTreeItem *>(index.internalPointer());

    return item->data(index.column());
}

/// Returns the item flags for the given model index
Qt::ItemFlags CategoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return QAbstractItemModel::flags(index);
}

/// Returns the appropriate header column (for horizontal DisplayRole)
QVariant CategoryTreeModel::headerData(int section,
                                       Qt::Orientation orientation,
                                       int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole &&
        section >= 0 && section < columnCount()) {
        return oRootItem->data(section);
    }

    return QVariant();
}

/*!
 * Returns index for given row and column of parent.
 * Determines the categoryTreeItem for the parent, uses 'row' to select a child,
 * then creates an index for the child item.
 * (Why? What's this achieve??)
 */
QModelIndex
CategoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    const CategoryTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = oRootItem;
    else
        parentItem = static_cast<CategoryTreeItem *>(parent.internalPointer());

    CategoryTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);

    return QModelIndex();
}

/// Finds the index for the parent of the given index
QModelIndex CategoryTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    CategoryTreeItem *childItem =
        static_cast<CategoryTreeItem *>(index.internalPointer());
    CategoryTreeItem *parentItem = childItem->parentItem();

    if (parentItem == oRootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

/// Determines number of rows for given model index
int CategoryTreeModel::rowCount(const QModelIndex &parent) const
{
    const CategoryTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = oRootItem;
    else
        parentItem = static_cast<CategoryTreeItem *>(parent.internalPointer());

    return parentItem->childCount();
}

/// Determines number of columns for given model index
int CategoryTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<CategoryTreeItem *>(parent.internalPointer())
            ->columnCount();

    return oRootItem->columnCount();
}

/// Detach model while updating contents
void CategoryTreeModel::startUpdate()
{
    beginResetModel();
}

/// Reattach model after updating contents
void CategoryTreeModel::endUpdate()
{
    endResetModel();
}

/*!
 * Adds an eix category to the tree.
 * categoryIndex: - entry number, 0+, in the eix collection of categories.
 * categoryName: - the name of the category, which by convention usually
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
        QVector<QVariant> node;
        node << part1 << QVariant::fromValue(categorySize)
             << QVariant::fromValue(categoryIndex);

        (void)oAllItem->appendChild(node);
    } else {
        CategoryTreeItem *top = oAllItem->findChild(part1);
        if (!top) {
            QVector<QVariant> topNode;
            topNode << part1 << 0 << -1;

            top = oAllItem->appendChild(topNode);
        }

        QVector<QVariant> node;
        node << part2 << QVariant::fromValue(categorySize)
             << QVariant::fromValue(categoryIndex);

        (void)top->appendChild(node);
        top->setPackageCount(top->packageCount() + categorySize);
    }
    oAllItem->setPackageCount(oAllItem->packageCount() + categorySize);
}

/// Clear the tree data - leave the root item (headers) and the "All" item
void CategoryTreeModel::clear()
{
    oAllItem->freeChildItems();
    oAllItem->setData(CategoryTreeItem::Column::PkgCount, 0);
}

const CategoryTreeItem *CategoryTreeModel::allItem() const
{
    return oAllItem;
}
