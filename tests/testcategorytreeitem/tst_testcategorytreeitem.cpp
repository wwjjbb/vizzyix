// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include <QtTest>

#include "categorytreeitem.h"

class TestCategoryTreeItem : public QObject
{
    Q_OBJECT

  public:
    TestCategoryTreeItem();
    ~TestCategoryTreeItem();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();
    void test_construction_data();
    void test_appendChild();
    void test_childCount();
    void test_freeChildItems();
    void test_setData();
    void test_row();
    void test_addCount();
    void test_isContainer();
    void test_categoryNumber();
    void test_findChild();
};

TestCategoryTreeItem::TestCategoryTreeItem()
{
}

TestCategoryTreeItem::~TestCategoryTreeItem()
{
}

void TestCategoryTreeItem::initTestCase()
{
}

void TestCategoryTreeItem::cleanupTestCase()
{
}

void TestCategoryTreeItem::cleanup()
{
}

void TestCategoryTreeItem::test_construction_data()
{
    CategoryTreeItem *first = CategoryTreeItem::newRootItem({"abc", 2, 3});

    QCOMPARE(first->columnCount(), 3);
    QCOMPARE(first->data(0), QVariant("abc"));
    QCOMPARE(first->data(1), QVariant(2));
    QCOMPARE(first->data(2), QVariant(3));

    QCOMPARE(first->data(-1), QVariant());
    QCOMPARE(first->data(3), QVariant());

    QCOMPARE(first->parentItem(), nullptr);

    CategoryTreeItem *second = CategoryTreeItem::newRootItem({"def", 2, 1});
    QCOMPARE(second->data(0), QVariant("def"));
    QCOMPARE(second->data(1), QVariant(2));
    QCOMPARE(second->data(2), QVariant(1));

    QCOMPARE(second->parentItem(), nullptr);

    delete first;
    delete second;
}

void TestCategoryTreeItem::test_appendChild()
{
    CategoryTreeItem *top = CategoryTreeItem::newRootItem({"def", 2, 3});
    QCOMPARE(top->childCount(), 0);

    for (int cn = 0; cn < 10; ++cn) {
        CategoryTreeItem *child =
            top->appendChild({QString("TEST_%1").arg(cn), 8, 9});
        QCOMPARE(top->childCount(), cn + 1);
        QCOMPARE(child->parentItem(), top);

        for (int gcn = 0; gcn < 10; ++gcn) {
            CategoryTreeItem *grandchild =
                child->appendChild({QString("GC_%1").arg(gcn), gcn, gcn * 2});
            QCOMPARE(child->childCount(), gcn + 1);
            QCOMPARE(grandchild->parentItem(), child);
        }

        QCOMPARE(top->childCount(), cn + 1);
        QCOMPARE(child->parentItem(), top);
    }
    delete top;
}

void TestCategoryTreeItem::test_childCount()
{
}

void TestCategoryTreeItem::test_freeChildItems()
{
    CategoryTreeItem *top = CategoryTreeItem::newRootItem({"abc", 2, 3});
    QCOMPARE(top->childCount(), 0);

    for (int n = 0; n < 10; ++n) {
        (void)top->appendChild({QString("TEST_%1").arg(n), 8, 9});
    }

    QCOMPARE(top->childCount(), 10);
    top->freeChildItems();
    QCOMPARE(top->childCount(), 0);

    delete top;
}

void TestCategoryTreeItem::test_setData()
{
    CategoryTreeItem *top = CategoryTreeItem::newRootItem({"abc", 2, 3});

    QCOMPARE(top->data(0), QVariant("abc"));
    QCOMPARE(top->data(1), QVariant(2));
    QCOMPARE(top->data(2), QVariant(3));

    top->setData(1, 10);
    QCOMPARE(top->data(0), QVariant("abc"));
    QCOMPARE(top->data(1), QVariant(10));
    QCOMPARE(top->data(2), QVariant(3));

    top->setData(0, "def");
    QCOMPARE(top->data(0), QVariant("def"));
    QCOMPARE(top->data(1), QVariant(10));
    QCOMPARE(top->data(2), QVariant(3));

    top->setData(2, 42);
    QCOMPARE(top->data(0), QVariant("def"));
    QCOMPARE(top->data(1), QVariant(10));
    QCOMPARE(top->data(2), QVariant(42));

    // Should not crash on out of range either
    top->setData(3, 420);
    QCOMPARE(top->data(0), QVariant("def"));
    QCOMPARE(top->data(1), QVariant(10));
    QCOMPARE(top->data(2), QVariant(42));

    top->setData(-1, 83);
    QCOMPARE(top->data(0), QVariant("def"));
    QCOMPARE(top->data(1), QVariant(10));
    QCOMPARE(top->data(2), QVariant(42));

    delete top;
}

void TestCategoryTreeItem::test_row()
{
    CategoryTreeItem *top = CategoryTreeItem::newRootItem({"abc", 2, 3});
    QCOMPARE(top->childCount(), 0);

    for (int n = 0; n < 10; ++n) {
        (void)top->appendChild({QString("TEST_%1").arg(n), 8, 9});
    }
    QCOMPARE(top->childCount(), 10);

    for (int n = 0; n < 10; ++n) {
        CategoryTreeItem *item = top->child(n);
        QCOMPARE(item->row(), n);
        QCOMPARE(item->parentItem(), top);
    }

    // check out of range child numbers while we're here
    QCOMPARE(top->child(-1), nullptr);
    QCOMPARE(top->child(10), nullptr);

    delete top;
}

void TestCategoryTreeItem::test_addCount()
{
    CategoryTreeItem *top = CategoryTreeItem::newRootItem({"abc", 2, 3});

    QCOMPARE(top->data(CategoryTreeItem::Column::Name), QVariant("abc"));
    QCOMPARE(top->data(CategoryTreeItem::Column::PkgCount), QVariant(2));
    QCOMPARE(top->data(CategoryTreeItem::Column::CatIndex), QVariant(3));
    QCOMPARE(top->packageCount(), 2u);

    top->setPackageCount(17);
    QCOMPARE(top->data(CategoryTreeItem::Column::Name), QVariant("abc"));
    QCOMPARE(top->data(CategoryTreeItem::Column::PkgCount), QVariant(17));
    QCOMPARE(top->data(CategoryTreeItem::Column::CatIndex), QVariant(3));

    delete top;
}

void TestCategoryTreeItem::test_isContainer()
{
    CategoryTreeItem *top = CategoryTreeItem::newRootItem({"abc", 2, -1});
    CategoryTreeItem *child = top->appendChild({"abc", 2, 3});

    QVERIFY(top->isContainer());
    QVERIFY(!child->isContainer());

    delete top;
}

void TestCategoryTreeItem::test_categoryNumber()
{
    CategoryTreeItem *top = CategoryTreeItem::newRootItem({"abc", 2, -1});

    QCOMPARE(top->categoryNumber(), -1);

    delete top;
}

void TestCategoryTreeItem::test_findChild()
{
    CategoryTreeItem *top = CategoryTreeItem::newRootItem({"def", 2, 3});
    QCOMPARE(top->childCount(), 0);

    for (int n = 0; n < 10; ++n) {
        CategoryTreeItem *child =
            top->appendChild({QString("TEST_%1").arg(n), 8, 9});
        QCOMPARE(top->childCount(), n + 1);
        QCOMPARE(child->parentItem(), top);
    }

    for (int n = 0; n < 10; ++n) {
        QString name(QString("TEST_%1").arg(n));

        CategoryTreeItem *found = top->findChild(name);
        QCOMPARE(found, top->child(n));
    }

    CategoryTreeItem *nope = top->findChild(QString("Fred"));
    QCOMPARE(nope, nullptr);

    delete top;
}

QTEST_APPLESS_MAIN(TestCategoryTreeItem)

#include "tst_testcategorytreeitem.moc"
