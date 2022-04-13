// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include <QtTest>

#include "combinedpackageinfo.h"
#include <QDir>
#include <QString>

class TestCombinedPackageInfo : public QObject
{
    Q_OBJECT

  public:
    TestCombinedPackageInfo();
    ~TestCombinedPackageInfo();

  private:
    CombinedPackageInfo *testItem = nullptr;

    QDir testDir = QDir("/aaa/bbb/ccc");
    QDir testDirA = QDir("/aaa/aaa/aaa");
    QDir testDirB = QDir("/bbb/bbb/bbb");
    QDir testDirC = QDir("/ccc/ccc/ccc");
    QDir testDirD = QDir("/ddd/ddd/ddd");

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_setup();
    void test_construction();
    void test_copy_constructor();
    void test_swap();
    void test_assignment();
    void test_setStateDb();
};

TestCombinedPackageInfo::TestCombinedPackageInfo()
{
}

TestCombinedPackageInfo::~TestCombinedPackageInfo()
{
}

void TestCombinedPackageInfo::initTestCase()
{
    testItem = new CombinedPackageInfo("1.2.3", testDir);
}

void TestCombinedPackageInfo::cleanupTestCase()
{
    delete testItem;
}

void TestCombinedPackageInfo::test_setup()
{
    QCOMPARE(testItem->versionName(), "1.2.3");
    QCOMPARE(testItem->versionDir(), testDir);
    QCOMPARE(testItem->inEixDb(), false);
    QCOMPARE(testItem->inPkgDb(), false);
}

void TestCombinedPackageInfo::test_construction()
{
    CombinedPackageInfo a("aaa", testDirA);
    QCOMPARE(a.versionName(), "aaa");
    QCOMPARE(a.versionDir(), testDirA);
    QCOMPARE(a.inEixDb(), false);
    QCOMPARE(a.inPkgDb(), false);

    CombinedPackageInfo b("bbb", testDirB);
    QCOMPARE(b.versionName(), "bbb");
    QCOMPARE(b.versionDir(), testDirB);
    QCOMPARE(b.inEixDb(), false);
    QCOMPARE(b.inPkgDb(), false);
}

void TestCombinedPackageInfo::test_copy_constructor()
{
    CombinedPackageInfo aaa("aaa", testDirA);
    CombinedPackageInfo a(aaa);
    QCOMPARE(a.versionName(), "aaa");
    QCOMPARE(a.versionDir(), testDirA);
    QCOMPARE(a.inEixDb(), false);
    QCOMPARE(a.inPkgDb(), false);

    CombinedPackageInfo bbb("bbb", testDirB);
    bbb.setPkgDb(true);
    CombinedPackageInfo b(bbb);
    QCOMPARE(b.versionName(), "bbb");
    QCOMPARE(b.versionDir(), testDirB);
    QCOMPARE(b.inEixDb(), false);
    QCOMPARE(b.inPkgDb(), true);

    CombinedPackageInfo ccc("ccc", testDirC);
    ccc.setEixDb(true);
    CombinedPackageInfo c(ccc);
    QCOMPARE(c.versionName(), "ccc");
    QCOMPARE(c.versionDir(), testDirC);
    QCOMPARE(c.inEixDb(), true);
    QCOMPARE(c.inPkgDb(), false);

    CombinedPackageInfo ddd("ddd", testDirD);
    ddd.setEixDb(true);
    ddd.setPkgDb(true);
    CombinedPackageInfo d(ddd);
    QCOMPARE(d.versionName(), "ddd");
    QCOMPARE(d.versionDir(), testDirD);
    QCOMPARE(d.inEixDb(), true);
    QCOMPARE(d.inPkgDb(), true);
}

void TestCombinedPackageInfo::test_swap()
{
    CombinedPackageInfo b("bbb", testDirB);
    b.setPkgDb(true);
    QCOMPARE(b.versionName(), "bbb");
    QCOMPARE(b.versionDir(), testDirB);
    QCOMPARE(b.inEixDb(), false);
    QCOMPARE(b.inPkgDb(), true);

    CombinedPackageInfo c("ccc", testDirC);
    c.setEixDb(true);
    QCOMPARE(c.versionName(), "ccc");
    QCOMPARE(c.versionDir(), testDirC);
    QCOMPARE(c.inEixDb(), true);
    QCOMPARE(c.inPkgDb(), false);

    swap(b, c);

    QCOMPARE(c.versionName(), "bbb");
    QCOMPARE(c.versionDir(), testDirB);
    QCOMPARE(c.inEixDb(), false);
    QCOMPARE(c.inPkgDb(), true);

    QCOMPARE(b.versionName(), "ccc");
    QCOMPARE(b.versionDir(), testDirC);
    QCOMPARE(b.inEixDb(), true);
    QCOMPARE(b.inPkgDb(), false);
}

void TestCombinedPackageInfo::test_assignment()
{
    CombinedPackageInfo b("bbb", testDirB);
    b.setPkgDb(true);
    CombinedPackageInfo c("ccc", testDirC);
    c.setEixDb(true);

    b = c;

    QCOMPARE(b.versionName(), "ccc");
    QCOMPARE(b.versionDir(), testDirC);
    QCOMPARE(b.inEixDb(), true);
    QCOMPARE(b.inPkgDb(), false);
}

void TestCombinedPackageInfo::test_setStateDb()
{
    CombinedPackageInfo b("bbb", testDirB);
    QCOMPARE(b.inEixDb(), false);
    QCOMPARE(b.inPkgDb(), false);
    QCOMPARE(b.versionDir(), testDirB);

    b.setEixDb(true);
    b.setPkgDb(false);
    b.setVersionDir(testDirA);
    QCOMPARE(b.inEixDb(), true);
    QCOMPARE(b.inPkgDb(), false);
    QCOMPARE(b.versionDir(), testDirA);

    b.setEixDb(true);
    b.setPkgDb(true);
    b.setVersionDir(testDirC);
    QCOMPARE(b.inEixDb(), true);
    QCOMPARE(b.inPkgDb(), true);
    QCOMPARE(b.versionDir(), testDirC);

    b.setEixDb(false);
    b.setPkgDb(false);
    b.setVersionDir(testDirD);
    QCOMPARE(b.inEixDb(), false);
    QCOMPARE(b.inPkgDb(), false);
    QCOMPARE(b.versionDir(), testDirD);

    b.setEixDb(false);
    b.setPkgDb(true);
    b.setVersionDir(testDir);
    QCOMPARE(b.inEixDb(), false);
    QCOMPARE(b.inPkgDb(), true);
    QCOMPARE(b.versionDir(), testDir);
}

QTEST_APPLESS_MAIN(TestCombinedPackageInfo)

#include "tst_testcombinedpackageinfo.moc"
