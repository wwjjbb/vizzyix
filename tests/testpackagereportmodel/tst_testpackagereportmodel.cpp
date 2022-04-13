// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include <QtTest>

#include "eix.pb.h"
#include "packagereportmodel.h"
#include <fstream>

class testpackagereportmodel : public QObject
{
    Q_OBJECT

  public:
    testpackagereportmodel();
    ~testpackagereportmodel();

  private slots:
    void initTestCase();
    void test_construction();
    void test_headerData();
    void test_addPackage();
    void test_clear();
    void test_data_fetch_data_role();
    void test_packageItem();

  private:
    int findCat(std::string catName);
    int findPkg(int catNumber, std::string pkgName);

    eix_proto::Collection eix;
    VersionMap emptyVersionList;
    int cat_dev_qt;
    int pkg_dev_qt_ww_qt_creator;
    int pkg_dev_qt_ww_qtcore;
};

testpackagereportmodel::testpackagereportmodel()
{
}

testpackagereportmodel::~testpackagereportmodel()
{
}

void testpackagereportmodel::initTestCase()
{
    std::fstream input(TESTDATA, std::ios::in | std::ios::binary);
    if (!eix.ParseFromIstream(&input)) {
        QFAIL("Failed to parse data file: " TESTDATA);
    } else {
        cat_dev_qt = findCat("dev-qt");
        QVERIFY(cat_dev_qt >= 0);
        pkg_dev_qt_ww_qt_creator = findPkg(cat_dev_qt, "qt-creator");
        QVERIFY(pkg_dev_qt_ww_qt_creator >= 0);
        pkg_dev_qt_ww_qtcore = findPkg(cat_dev_qt, "qtcore");
        QVERIFY(pkg_dev_qt_ww_qtcore >= 0);
    }
}

void testpackagereportmodel::test_construction()
{
    QModelIndex dummy;
    PackageReportModel something;

    QCOMPARE(something.rowCount(dummy), 0);
    QCOMPARE(something.columnCount(), 4);
}

void testpackagereportmodel::test_headerData()
{
    PackageReportModel something;

    // A reminder to keep the case statement in headerData() in sync with the
    // data cols
    QVERIFY(PackageReportItem::columnCount() == 4);

    QCOMPARE(something.headerData(PackageReportItem::Column::Installed,
                                  Qt::Horizontal),
             QVariant());
    QCOMPARE(
        something.headerData(PackageReportItem::Column::Name, Qt::Horizontal),
        QVariant("Package"));
    QCOMPARE(something.headerData(PackageReportItem::Column::Description,
                                  Qt::Horizontal),
             QVariant("Description"));
}

void testpackagereportmodel::test_addPackage()
{
    PackageReportModel something;

    const eix_proto::Category &cat = eix.category(cat_dev_qt);
    const eix_proto::Package &pkg1 = cat.package(pkg_dev_qt_ww_qt_creator);
    const eix_proto::Package &pkg2 = cat.package(pkg_dev_qt_ww_qtcore);

    something.addPackage(cat.category(), pkg1, emptyVersionList);
    QCOMPARE(something.rowCount(), 1);

    something.addPackage(cat.category(), pkg2, emptyVersionList);
    QCOMPARE(something.rowCount(), 2);
}

void testpackagereportmodel::test_clear()
{
    PackageReportModel something;

    const eix_proto::Category &cat = eix.category(cat_dev_qt);
    const eix_proto::Package &pkg1 = cat.package(pkg_dev_qt_ww_qt_creator);
    const eix_proto::Package &pkg2 = cat.package(pkg_dev_qt_ww_qtcore);

    something.addPackage(cat.category(), pkg1, emptyVersionList);
    something.addPackage(cat.category(), pkg2, emptyVersionList);
    QCOMPARE(something.rowCount(), 2);

    something.clear();
    QCOMPARE(something.rowCount(), 0);
}

void testpackagereportmodel::test_data_fetch_data_role()
{
    PackageReportModel something;

    const eix_proto::Category &cat = eix.category(cat_dev_qt);
    const eix_proto::Package &pkg1 = cat.package(pkg_dev_qt_ww_qt_creator);
    const eix_proto::Package &pkg2 = cat.package(pkg_dev_qt_ww_qtcore);

    something.addPackage(cat.category(), pkg1, emptyVersionList);
    something.addPackage(cat.category(), pkg2, emptyVersionList);

    QCOMPARE(
        something.data(something.index(0, PackageReportItem::Column::Installed),
                       Qt::DisplayRole),
        QVariant());
    QCOMPARE(something.data(something.index(0, PackageReportItem::Column::Name),
                            Qt::DisplayRole),
             QVariant("qt-creator"));
    QCOMPARE(something.data(
                 something.index(0, PackageReportItem::Column::Description),
                 Qt::DisplayRole),
             QVariant::fromValue(QString::fromStdString(pkg1.description())));

    QCOMPARE(
        something.data(something.index(1, PackageReportItem::Column::Installed),
                       Qt::DisplayRole),
        QVariant());
    QCOMPARE(something.data(something.index(1, PackageReportItem::Column::Name),
                            Qt::DisplayRole),
             QVariant("qtcore"));
    QCOMPARE(something.data(
                 something.index(1, PackageReportItem::Column::Description),
                 Qt::DisplayRole),
             QVariant::fromValue(QString::fromStdString(pkg2.description())));
}

void testpackagereportmodel::test_packageItem()
{
    PackageReportModel something;

    const eix_proto::Category &cat = eix.category(cat_dev_qt);
    const eix_proto::Package &pkg1 = cat.package(pkg_dev_qt_ww_qt_creator);
    const eix_proto::Package &pkg2 = cat.package(pkg_dev_qt_ww_qtcore);

    something.addPackage(cat.category(), pkg1, emptyVersionList);
    something.addPackage(cat.category(), pkg2, emptyVersionList);

    // Just a simple check on contents - the PackageReportItem values are copied
    // into the model's vector so would need to create an == operator to make
    // this work.
    QCOMPARE(something.packageItem(0).name(), "qt-creator");
    QCOMPARE(something.packageItem(1).name(), "qtcore");
}

int testpackagereportmodel::findCat(std::string catName)
{
    int catNumber;
    for (catNumber = 0; catNumber < eix.category_size(); ++catNumber) {
        if (catName == eix.category(catNumber).category()) {
            return catNumber;
        }
    }
    return -1;
}

int testpackagereportmodel::findPkg(int catNumber, std::string pkgName)
{
    int pkgNumber;

    if (catNumber >= 0 && catNumber < eix.category_size()) {
        const eix_proto::Category &cat = eix.category(catNumber);
        for (pkgNumber = 0; pkgNumber < cat.package_size(); ++pkgNumber) {
            if (pkgName == cat.package(pkgNumber).name()) {
                return pkgNumber;
            }
        }
    }
    return -1;
}

QTEST_APPLESS_MAIN(testpackagereportmodel)

#include "tst_testpackagereportmodel.moc"
