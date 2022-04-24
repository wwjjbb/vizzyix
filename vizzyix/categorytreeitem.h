// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#ifndef CATEGORYTREEITEM_H
#define CATEGORYTREEITEM_H

#include <QVariant>
#include <QVector>

class CategoryTreeItem
{
  public:
    static CategoryTreeItem *newRootItem(const QVector<QVariant> &data);
    ~CategoryTreeItem();

  private:
    // Disallow copy/swap/assign because they make no sense for a tree item
    // which always has to belong to a tree.
    CategoryTreeItem(const CategoryTreeItem &other);
    friend void swap(CategoryTreeItem &first, CategoryTreeItem &second);
    CategoryTreeItem &operator=(CategoryTreeItem &other);

  public:
    CategoryTreeItem *appendChild(const QVector<QVariant> &data);
    void freeChildItems();

    CategoryTreeItem *child(int row) const;
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    void setData(int column, const QVariant value);
    int row() const;
    CategoryTreeItem *parentItem() const;

    uint packageCount() const;
    void setPackageCount(uint pkgCount);
    bool isContainer() const;
    int categoryNumber() const;

    CategoryTreeItem *findChild(const QString &childName) const;

    enum Column { Name, PkgCount, CatIndex };

  private:
    // Hide to disallow instances being be created on stack
    explicit CategoryTreeItem(const QVector<QVariant> &data,
                              CategoryTreeItem *parentItem = nullptr);

    QVector<CategoryTreeItem *> childItems_;
    QVector<QVariant> itemData_;
    // TODO - parentItem should be const but there are complications with that
    CategoryTreeItem *parentItem_;
};

#endif // CATEGORYTREEITEM_H
