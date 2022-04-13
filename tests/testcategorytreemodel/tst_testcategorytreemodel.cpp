// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include <QtTest>
#include <categorytreemodel.h>

class TestCategoryTreeModel : public QObject
{
    Q_OBJECT

  public:
    TestCategoryTreeModel();
    ~TestCategoryTreeModel();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void test_construction();
    void test_clear();
    void test_headerData();
    void test_addCategory();
    void test_index();
    void test_data();
    void test_parent();
    void test_rowCount();
    void test_columnCount();

  private:
    void setupTree();
    CategoryTreeModel *base;
};

TestCategoryTreeModel::TestCategoryTreeModel()
{
}

TestCategoryTreeModel::~TestCategoryTreeModel()
{
}

void TestCategoryTreeModel::initTestCase()
{
    base = new CategoryTreeModel();
}

void TestCategoryTreeModel::cleanupTestCase()
{
    delete base;
}

void TestCategoryTreeModel::cleanup()
{
    base->clear();
}

void TestCategoryTreeModel::test_construction()
{
    QCOMPARE(base->columnCount(), 3);

    // Starts with a single row: the "All" node
    QCOMPARE(base->rowCount(), 1);
}

void TestCategoryTreeModel::test_clear()
{
    CategoryTreeModel stack;
    const CategoryTreeItem *all = stack.allItem();

    QCOMPARE(all->childCount(), 0);

    stack.addCategory(1, "fred", 3);
    stack.addCategory(2, "jim", 6);
    QCOMPARE(all->childCount(), 2);

    stack.clear();
    QCOMPARE(all->childCount(), 0);
}

void TestCategoryTreeModel::test_headerData()
{
    // the expected headings
    QCOMPARE(base->headerData(0, Qt::Horizontal), "Categories");
    QCOMPARE(base->headerData(1, Qt::Horizontal), "Pkgs");
    QCOMPARE(base->headerData(2, Qt::Horizontal), "Idx");

    // index out of range
    QCOMPARE(base->headerData(3, Qt::Horizontal), QVariant());
    QCOMPARE(base->headerData(-1, Qt::Horizontal), QVariant());

    // vertical
    QCOMPARE(base->headerData(0, Qt::Vertical), QVariant());

    // Check the initial state of the "All" node
    QModelIndex allCat = base->index(0, 0);
    QCOMPARE(base->data(allCat, Qt::DisplayRole), "All");

    QModelIndex allPackages = base->index(0, 1);
    QCOMPARE(base->data(allPackages, Qt::DisplayRole), 0);

    QModelIndex allIdx = base->index(0, 2);
    QCOMPARE(base->data(allIdx, Qt::DisplayRole), -1);
}

void TestCategoryTreeModel::test_addCategory()
{
    // Check the details using the CategoryTreeItem tree representation.

    const CategoryTreeItem *all = base->allItem();
    QCOMPARE(all->childCount(), 0);
    QCOMPARE(all->packageCount(), 0u);

    // Add "First-One"

    base->addCategory(1, "First-One", 41);
    QCOMPARE(all->childCount(), 1);
    QCOMPARE(all->packageCount(), 41u);

    const CategoryTreeItem *first = all->child(0);
    QCOMPARE(first->data(CategoryTreeItem::Column::Name), "First");
    QCOMPARE(first->childCount(), 1);
    QCOMPARE(first->packageCount(), 41u);

    const CategoryTreeItem *firstOne = first->child(0);
    QCOMPARE(firstOne->data(CategoryTreeItem::Column::Name), "One");
    QCOMPARE(firstOne->childCount(), 0);
    QCOMPARE(firstOne->packageCount(), 41u);

    // Add "First-Two"

    base->addCategory(2, "First-Two", 42);
    QCOMPARE(all->childCount(), 1);
    QCOMPARE(all->packageCount(), 83u);

    QCOMPARE(first->data(CategoryTreeItem::Column::Name), "First");
    QCOMPARE(first->childCount(), 2);
    QCOMPARE(first->packageCount(), 83u);

    const CategoryTreeItem *firstTwo = first->child(1);
    QCOMPARE(firstTwo->data(CategoryTreeItem::Column::Name), "Two");
    QCOMPARE(firstTwo->childCount(), 0);
    QCOMPARE(firstTwo->packageCount(), 42u);

    // Add "Second-One"
    base->addCategory(3, "Second-One", 43);
    QCOMPARE(all->childCount(), 2);
    QCOMPARE(all->packageCount(), 126u);

    const CategoryTreeItem *second = all->child(1);
    QCOMPARE(second->data(CategoryTreeItem::Column::Name), "Second");
    QCOMPARE(second->childCount(), 1);
    QCOMPARE(second->packageCount(), 43u);

    const CategoryTreeItem *secondOne = second->child(0);
    QCOMPARE(secondOne->data(CategoryTreeItem::Column::Name), "One");
    QCOMPARE(secondOne->childCount(), 0);
    QCOMPARE(secondOne->packageCount(), 43u);

    // Add "Second-Two"

    base->addCategory(4, "Second-Two", 44);
    QCOMPARE(all->childCount(), 2);
    QCOMPARE(all->packageCount(), 170u);

    QCOMPARE(second->data(CategoryTreeItem::Column::Name), "Second");
    QCOMPARE(second->childCount(), 2);
    QCOMPARE(second->packageCount(), 87u);

    const CategoryTreeItem *secondTwo = second->child(1);
    QCOMPARE(secondTwo->data(CategoryTreeItem::Column::Name), "Two");
    QCOMPARE(secondTwo->childCount(), 0);
    QCOMPARE(secondTwo->packageCount(), 44u);

    // Add "Third"
    base->addCategory(5, "Third", 45);
    QCOMPARE(all->childCount(), 3);
    QCOMPARE(all->packageCount(), 215u);

    const CategoryTreeItem *third = all->child(2);
    QCOMPARE(third->data(CategoryTreeItem::Column::Name), "Third");
    QCOMPARE(third->childCount(), 0);
    QCOMPARE(third->packageCount(), 45u);
}

void TestCategoryTreeModel::test_index()
{
    setupTree();

    // Use data() to get the name field for the index, and check the names are
    // as expected

    // Row 0 should be the all item
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::Name),
                        Qt::DisplayRole),
             "All");

    QModelIndex all = base->index(0, 0);
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::Name, all),
                        Qt::DisplayRole),
             "First");

    QModelIndex first = base->index(0, 0, all);
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::Name, first),
                        Qt::DisplayRole),
             "One");
    QCOMPARE(base->data(base->index(1, CategoryTreeItem::Column::Name, first),
                        Qt::DisplayRole),
             "Two");

    QCOMPARE(base->data(base->index(1, CategoryTreeItem::Column::Name, all),
                        Qt::DisplayRole),
             "Second");

    QModelIndex second = base->index(1, 0, all);
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::Name, second),
                        Qt::DisplayRole),
             "One");
    QCOMPARE(base->data(base->index(1, CategoryTreeItem::Column::Name, second),
                        Qt::DisplayRole),
             "Two");

    QCOMPARE(base->data(base->index(2, CategoryTreeItem::Column::Name, all),
                        Qt::DisplayRole),
             "Third");
}

void TestCategoryTreeModel::test_data()
{
    setupTree();

    // Row 0 is the all item
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::Name),
                        Qt::DisplayRole),
             "All");
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::CatIndex),
                        Qt::DisplayRole),
             -1);
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::PkgCount),
                        Qt::DisplayRole),
             215);

    QModelIndex all = base->index(0, 0);
    QModelIndex first = base->index(0, 0, all);
    QModelIndex second = base->index(1, 0, all);

    // Subcat: First
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::Name, all),
                        Qt::DisplayRole),
             "First");
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::CatIndex, all),
                        Qt::DisplayRole),
             -1);
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::PkgCount, all),
                        Qt::DisplayRole),
             83);

    // Subcat: Second
    QCOMPARE(base->data(base->index(1, CategoryTreeItem::Column::Name, all),
                        Qt::DisplayRole),
             "Second");
    QCOMPARE(base->data(base->index(1, CategoryTreeItem::Column::CatIndex, all),
                        Qt::DisplayRole),
             -1);
    QCOMPARE(base->data(base->index(1, CategoryTreeItem::Column::PkgCount, all),
                        Qt::DisplayRole),
             87);

    // Subcat: Third
    QCOMPARE(base->data(base->index(2, CategoryTreeItem::Column::Name, all),
                        Qt::DisplayRole),
             "Third");
    QCOMPARE(base->data(base->index(2, CategoryTreeItem::Column::CatIndex, all),
                        Qt::DisplayRole),
             5);
    QCOMPARE(base->data(base->index(2, CategoryTreeItem::Column::PkgCount, all),
                        Qt::DisplayRole),
             45);

    // Second level

    // Subcat: First-One
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::Name, first),
                        Qt::DisplayRole),
             "One");
    QCOMPARE(
        base->data(base->index(0, CategoryTreeItem::Column::CatIndex, first),
                   Qt::DisplayRole),
        1);
    QCOMPARE(
        base->data(base->index(0, CategoryTreeItem::Column::PkgCount, first),
                   Qt::DisplayRole),
        41);

    // Subcat: First-Two
    QCOMPARE(base->data(base->index(1, CategoryTreeItem::Column::Name, first),
                        Qt::DisplayRole),
             "Two");
    QCOMPARE(
        base->data(base->index(1, CategoryTreeItem::Column::CatIndex, first),
                   Qt::DisplayRole),
        2);
    QCOMPARE(
        base->data(base->index(1, CategoryTreeItem::Column::PkgCount, first),
                   Qt::DisplayRole),
        42);

    // Subcat: Second-One
    QCOMPARE(base->data(base->index(0, CategoryTreeItem::Column::Name, second),
                        Qt::DisplayRole),
             "One");
    QCOMPARE(
        base->data(base->index(0, CategoryTreeItem::Column::CatIndex, second),
                   Qt::DisplayRole),
        3);
    QCOMPARE(
        base->data(base->index(0, CategoryTreeItem::Column::PkgCount, second),
                   Qt::DisplayRole),
        43);

    // Subcat: Second-Two
    QCOMPARE(base->data(base->index(1, CategoryTreeItem::Column::Name, second),
                        Qt::DisplayRole),
             "Two");
    QCOMPARE(
        base->data(base->index(1, CategoryTreeItem::Column::CatIndex, second),
                   Qt::DisplayRole),
        4);
    QCOMPARE(
        base->data(base->index(1, CategoryTreeItem::Column::PkgCount, second),
                   Qt::DisplayRole),
        44);
}

void TestCategoryTreeModel::test_parent()
{
    setupTree();

    QModelIndex all = base->index(0, 0);
    QModelIndex first = base->index(0, 0, all);
    QModelIndex firstOne = base->index(0, 0, first);
    QModelIndex firstTwo = base->index(1, 0, first);
    QModelIndex second = base->index(1, 0, all);
    QModelIndex secondOne = base->index(0, 0, second);
    QModelIndex secondTwo = base->index(1, 0, second);
    QModelIndex third = base->index(2, 0, all);

    QCOMPARE(base->parent(all), QModelIndex());

    QCOMPARE(base->parent(first), all);
    QCOMPARE(base->parent(second), all);
    QCOMPARE(base->parent(third), all);

    QCOMPARE(base->parent(firstOne), first);
    QCOMPARE(base->parent(firstTwo), first);

    QCOMPARE(base->parent(secondOne), second);
    QCOMPARE(base->parent(secondTwo), second);
}

void TestCategoryTreeModel::test_rowCount()
{
    QModelIndex all = base->index(0, 0);
    QCOMPARE(base->rowCount(all), 0);

    setupTree();

    QModelIndex first = base->index(0, 0, all);
    QModelIndex firstOne = base->index(0, 0, first);
    QModelIndex firstTwo = base->index(1, 0, first);
    QModelIndex second = base->index(1, 0, all);
    QModelIndex secondOne = base->index(0, 0, second);
    QModelIndex secondTwo = base->index(1, 0, second);
    QModelIndex third = base->index(2, 0, all);

    QCOMPARE(base->rowCount(), 1);

    const CategoryTreeItem *allItem = base->allItem();
    QCOMPARE(allItem->childCount(), 3);

    QCOMPARE(base->rowCount(all), 3);

    QCOMPARE(base->rowCount(first), 2);
    QCOMPARE(base->rowCount(firstOne), 0);
    QCOMPARE(base->rowCount(firstTwo), 0);
    QCOMPARE(base->rowCount(second), 2);
    QCOMPARE(base->rowCount(secondOne), 0);
    QCOMPARE(base->rowCount(secondTwo), 0);
    QCOMPARE(base->rowCount(third), 0);
}

void TestCategoryTreeModel::test_columnCount()
{
    setupTree();

    QModelIndex all = base->index(0, 0);
    QModelIndex first = base->index(0, 0, all);
    QModelIndex firstOne = base->index(0, 0, first);
    QModelIndex firstTwo = base->index(1, 0, first);
    QModelIndex second = base->index(1, 0, all);
    QModelIndex secondOne = base->index(0, 0, second);
    QModelIndex secondTwo = base->index(1, 0, second);
    QModelIndex third = base->index(2, 0, all);

    QCOMPARE(base->columnCount(), 3);
    QCOMPARE(base->columnCount(all), 3);
    QCOMPARE(base->columnCount(first), 3);
    QCOMPARE(base->columnCount(firstOne), 3);
    QCOMPARE(base->columnCount(firstTwo), 3);
    QCOMPARE(base->columnCount(second), 3);
    QCOMPARE(base->columnCount(secondOne), 3);
    QCOMPARE(base->columnCount(secondTwo), 3);
    QCOMPARE(base->columnCount(third), 3);
}

void TestCategoryTreeModel::setupTree()
{
    base->addCategory(1, "First-One", 41);
    base->addCategory(2, "First-Two", 42);
    base->addCategory(3, "Second-One", 43);
    base->addCategory(4, "Second-Two", 44);
    base->addCategory(5, "Third", 45);
}

QTEST_APPLESS_MAIN(TestCategoryTreeModel)

#include "tst_testcategorytreemodel.moc"
