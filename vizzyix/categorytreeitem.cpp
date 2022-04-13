// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "categorytreeitem.h"

/*
 * CategoryTreeItem::CategoryTreeItem
 *
 * Constructor
 *
 * Private to ensure that the appropriate factory is used to create new
 * instances, thereby ensuring the parent field is always set correctly.
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
    : itemData_(data), parentItem_(parentItem)
{
}

/*
 * CategoryTreeItem::newRootItem
 *
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

/*
 * CategoryTreeItem::~CategoryTreeItem
 *
 * Destructor
 *
 * Releases all child items.
 */
CategoryTreeItem::~CategoryTreeItem()
{
    freeChildItems();
}

/*
 * CategoryTreeItem::CategoryTreeItem
 *
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
    : childItems_(other.childItems_), itemData_(other.itemData_),
      parentItem_(other.parentItem_)
{
}

/*
 * swap
 *
 * Dummy placeholder
 *
 * This is private to ensure it's not used - the parents would need to be
 * updated to make this preserve the invariants.
 */
void swap(CategoryTreeItem &, CategoryTreeItem &)
{
}

/*
 * CategoryTreeItem::operator =
 *
 * Dummy placeholder
 *
 * This is private to ensure it's not used.
 *
 * Returns:
 *     reference to self
 */
CategoryTreeItem &CategoryTreeItem::operator=(CategoryTreeItem &)
{
    return *this;
}

/*
 * CategoryTreeItem::appendChild
 *
 * A factory routine which creates a new child node
 *
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
    auto child = new CategoryTreeItem(data, this);
    childItems_.append(child);
    return child;
}

/*
 * CategoryTreeItem::freeChildItems
 *
 * Frees all child nodes belonging to this object
 *
 * All of the child items are deleted, and the list is emptied.
 */
void CategoryTreeItem::freeChildItems()
{
    qDeleteAll(childItems_);
    childItems_.clear();
}

/*
 * CategoryTreeItem::child
 *
 * Finds the nth child item
 *
 * Given a row number, returns a pointer to the child tree item if row >= 0 and
 * row <childCount(). Otherwise returns a null instead.
 *
 * row:
 *     The index of the child to be located
 *
 * Returns:
 *     A pointer to the child item, or nullptr if the row number was out of
 * range.
 *
 */
CategoryTreeItem *CategoryTreeItem::child(int row) const
{
    if (row < 0 || row >= childItems_.count())
        return nullptr;

    return childItems_.at(row);
}

/*
 * CategoryTreeItem::childCount
 *
 * Returns the number of child items owned by this item.
 *
 * This is the number of rows for this node in the tree model.
 *
 * Returns:
 *     The number of child items
 *
 */
int CategoryTreeItem::childCount() const
{
    return childItems_.count();
}

/*
 * CategoryTreeItem::columnCount
 *
 * Returns the number of columms in the data.
 *
 * All the tree nodes have the same number of columns - Qt treats as a table.
 *
 * Return:
 *     Number of data columns
 */
int CategoryTreeItem::columnCount() const
{
    return itemData_.count();
}

/*
 * CategoryTreeItem::data
 *
 * Returns the data for the given column
 *
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
    if (column < 0 || column >= itemData_.size())
        return QVariant();
    return itemData_.at(column);
}

/*
 * CategoryTreeItem::setData
 *
 * Sets the value of given column
 *
 * Sets the value. The value is ignored if the column number is out of range.
 *
 * column:
 *     The column number to be set
 *
 * value:
 *     The new value of the column
 */
void CategoryTreeItem::setData(int column, const QVariant value)
{
    if (column >= 0 && column < itemData_.size()) {
        itemData_[column] = value;
    }
}

/*
 * CategoryTreeItem::row
 *
 * Gets the object;s row number within the parent
 *
 * The row number is the object's index in the parent's child list.
 *
 * Return:
 *     The row number
 */
int CategoryTreeItem::row() const
{
    if (parentItem_)
        return parentItem_->childItems_.indexOf(
            const_cast<CategoryTreeItem *>(this));

    return 0;
}

/*
 * CategoryTreeItem::parentItem
 *
 * Identifies the parent of this node
 *
 * The parent of the tree's root is a null pointer. All other nodes will
 * have an actual parent.
 *
 * Returns:
 *     A pointer to the parent item
 */
CategoryTreeItem *CategoryTreeItem::parentItem() const
{
    return parentItem_;
}

/*
 * CategoryTreeItem::packageCount
 *
 * Returns:
 *     The number of packages owned by this item
 */
uint CategoryTreeItem::packageCount() const
{
    return data(CategoryTreeItem::Column::PkgCount).toInt();
}

/*
 * CategoryTreeItem::setPackageCount
 *
 * Sets the number of packages owned by this node
 *
 * pkgCount:
 *     The number of packages
 */
void CategoryTreeItem::setPackageCount(uint pkgCount)
{
    setData(CategoryTreeItem::Column::PkgCount, QVariant::fromValue(pkgCount));
}

/*
 * CategoryTreeItem::isContainer
 *
 * Whether this node is a container
 *
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

/*
 *  CategoryTreeItem::categoryNumber
 *
 *  Get the category number of this node.
 *
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

/*
 * CategoryTreeItem::findChild
 *
 * Looks for the given name in child list and returns the child
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
