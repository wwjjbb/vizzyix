// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "categorytreeitem.h"

/*!
 * Private constructor to ensure that the appropriate factory is used to create
 * new instances, thereby ensuring the parent field is always set correctly.
 *
 * Constructs an instance of the class. All field values are provided by the
 * matching parameters.
 *
 * data:
 *     The object's data. It's a vector of variant because that's what Qt uses
 *     for data models, so it's easier this way.
 *
 * parentItem:
 *    The parent of this object; this value is only null for a root node. The
 *    parent owns this object and is responsible for managing the lifetime.
 */
CategoryTreeItem::CategoryTreeItem(const QVector<QVariant> &data,
                                   CategoryTreeItem *parentItem)
    : _itemData(data), _parentItem(parentItem)
{
}

/*!
 * A static factory routine to generate a root node.
 *
 * data:
 *     A variant list providing the headings for the model
 *
 * Returns:
 *     A new root node (parent is null)
 */
CategoryTreeItem *CategoryTreeItem::newRootItem(const QVector<QVariant> &data)
{
    return new CategoryTreeItem(data);
}

/*!
 * Destructor, releases all child items.
 */
CategoryTreeItem::~CategoryTreeItem()
{
    freeChildItems();
}

/*!
 * Copy constructor
 *
 * other:
 *     The object to be copied
 *
 * This is private to ensure that copying a tree item is only possible
 * within the class. The constraint is that a parent has references to
 * each of it's children.
 */
CategoryTreeItem::CategoryTreeItem(const CategoryTreeItem &other)
    : _childItems(other._childItems), _itemData(other._itemData),
      _parentItem(other._parentItem)
{
}

/*!
 * Dummy placeholder.
 * This is private to ensure it's not used - the parents would need to be
 * updated to make this preserve the invariants.
 */
void swap(CategoryTreeItem &, CategoryTreeItem &)
{
}

/*!
 * Dummy placeholder.
 * This is private to ensure it's not used.
 *
 * Returns:
 *     reference to self
 */
CategoryTreeItem &CategoryTreeItem::operator=(CategoryTreeItem &)
{
    return *this;
}

/*!
 * A factory routine which creates a new child node.
 * Adds a new child item with the given data to this object. This is the only
 * supported way to add a child item.
 *
 * data:
 *     A list of variants providing the data for the new child
 *
 * Returns:
 *     A pointer to the new child item
 *
 */
CategoryTreeItem *CategoryTreeItem::appendChild(const QVector<QVariant> &data)
{
    auto newChild = new CategoryTreeItem(data, this);
    _childItems.append(newChild);
    return newChild;
}

/*!
 * Frees all child nodes belonging to this object
 * All of the child items are deleted, and the list is emptied.
 */
void CategoryTreeItem::freeChildItems()
{
    qDeleteAll(_childItems);
    _childItems.clear();
}

/*!
 * Finds the nth child item.
 * Given a row number, returns a pointer to the child tree item if row >= 0 and
 * row <childCount(). Otherwise returns a null instead.
 *
 * row:
 *     The index of the child to be located
 *
 * Returns:
 *     A pointer to child item, or nullptr if the row number was out of range.
 *
 */
CategoryTreeItem *CategoryTreeItem::child(int row) const
{
    if (row < 0 || row >= _childItems.count())
        return nullptr;

    return _childItems.at(row);
}

/*!
 * Returns the number of child items owned by this item.
 * This is the number of rows for this node in the tree model.
 *
 * Returns:
 *     The number of child items
 *
 */
int CategoryTreeItem::childCount() const
{
    return _childItems.count();
}

/*!
 * Returns the number of columms in the data.
 * All the tree nodes have the same number of columns - Qt treats as a table.
 *
 * Return:
 *     Number of data columns
 */
int CategoryTreeItem::columnCount() const
{
    return _itemData.count();
}

/*!
 * Returns the data for the given column.
 * This is the actual value found, or an empty QVariant if the column number is
 * out of range.
 *
 * column:
 *     The column for which data is required
 *
 * Returns:
 *     The data value
 */
QVariant CategoryTreeItem::data(int column) const
{
    if (column < 0 || column >= _itemData.size())
        return QVariant();
    return _itemData.at(column);
}

/*!
 * Sets the value of given column.
 * The value is ignored if the column number is out of range.
 *
 * column:
 *     The column number to be set
 *
 * value:
 *     The new value of the column
 */
void CategoryTreeItem::setData(int column, const QVariant value)
{
    if (column >= 0 && column < _itemData.size()) {
        _itemData[column] = value;
    }
}

/*!
 * Gets the object's row number within the parent.
 * The row number is the object's index in the parent's child list.
 *
 * Return:
 *     The row number
 */
int CategoryTreeItem::row() const
{
    if (_parentItem)
        return _parentItem->_childItems.indexOf(
            const_cast<CategoryTreeItem *>(this));

    return 0;
}

/*!
 * Identifies the parent of this node.
 * The parent of the tree's root is a null pointer. All other nodes will
 * have an actual parent.
 *
 * Returns:
 *     A pointer to the parent item
 */
CategoryTreeItem *CategoryTreeItem::parentItem() const
{
    return _parentItem;
}

/// Returns the number of packages owned by this item
uint CategoryTreeItem::packageCount() const
{
    return data(CategoryTreeItem::Column::PkgCount).toInt();
}

/*!
 * Sets the number of packages owned by this node.
 *
 * pkgCount:
 *     The number of packages
 */
void CategoryTreeItem::setPackageCount(uint pkgCount)
{
    setData(CategoryTreeItem::Column::PkgCount, QVariant::fromValue(pkgCount));
}

/*!
 * Whether this node is a container.
 * Containers can contain other child items; those that can't are paired with an
 * eix category. e.g. sys-kernel/gentoo-sources is not a container but 'sys' is
 * and 'kernel' is.
 *
 * Returns:
 *     True if this is a container, otherwise false
 */
bool CategoryTreeItem::isContainer() const
{
    // Containers are groups of categories, not a single categeory.
    return categoryNumber() < 0;
}

/*!
 * Get the category number of this node.
 * The value is <0 for containers, or 0+ for the index of the category in the
 * eix data.
 *
 * Returns:
 *     The category number, or 0 for a container
 */
int CategoryTreeItem::categoryNumber() const
{
    return data(Column::CatIndex).toInt();
}

/*!
 * Looks for the given name in child list and returns the child.
 *
 * childName:
 *     The name to search for
 *
 * Returns:
 *     Pointer to child item, or null if not found
 */
CategoryTreeItem *CategoryTreeItem::findChild(const QString &childName) const
{
    QVariant required = childName;
    for (int r = 0; r < childCount(); ++r) {
        auto c = child(r);
        if (c->data(Column::Name) == required)
            return c;
    }
    return nullptr;
}
