// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include <QtTest>
#include <fstream>
#include <iostream>

#include "combinedpackageinfo.h"
#include "combinedpackagelist.h"
#include "eix.pb.h"
#include "packagereportitem.h"

class TestReportModelItem : public QObject
{
    Q_OBJECT

  public:
    TestReportModelItem();
    ~TestReportModelItem();

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void cleanup();

    void test_name();
    void test_description();
    void test_data_member();
    void test_data_member_with_999();
    void test_data_member_only_999();
    void test_copy_constructor();
    // TODO: void test_swap();
    // TODO: void test_assignment();
    void test_vector_of();
    void test_cached_values();

  private:
    int findCat(std::string catName);
    int findPkg(int catNumber, std::string pkgName);

    eix_proto::Collection eix;
    VersionMap emptyVersionMap;
    int cat_dev_qt;
    int pkg_dev_qt_ww_qt_creator;
    int pkg_dev_qt_ww_qtcore;
    int pkg_dev_qt_ww_qtdiag;

    int cat_sys_apps;
    int pkg_sys_apps_ww_wjbtools;

    int cat_app_accessibility;
    int pkg_app_accessibility_ww_emacspeak;
    int pkg_app_accessibility_ww_simon;
};

TestReportModelItem::TestReportModelItem()
{
}

TestReportModelItem::~TestReportModelItem()
{
}

void TestReportModelItem::initTestCase()
{
    std::fstream input(TESTDATA, std::ios::in | std::ios::binary);
    if (!eix.ParseFromIstream(&input)) {
        QFAIL("Failed to parse data file: " TESTDATA);
    } else {
        cat_dev_qt = findCat("dev-qt");
        QVERIFY(cat_dev_qt >= 0);

        cat_sys_apps = findCat("sys-apps");
        QVERIFY(cat_sys_apps >= 0);

        cat_app_accessibility = findCat("app-accessibility");
        QVERIFY(cat_app_accessibility >= 0);

        // world install
        pkg_dev_qt_ww_qt_creator = findPkg(cat_dev_qt, "qt-creator");
        QVERIFY(pkg_dev_qt_ww_qt_creator >= 0);

        // dependency install
        pkg_dev_qt_ww_qtcore = findPkg(cat_dev_qt, "qtcore");
        QVERIFY(pkg_dev_qt_ww_qtcore >= 0);

        // not installed
        pkg_dev_qt_ww_qtdiag = findPkg(cat_dev_qt, "qtdiag");
        QVERIFY(pkg_dev_qt_ww_qtdiag >= 0);

        // world install (overlay)
        pkg_sys_apps_ww_wjbtools = findPkg(cat_sys_apps, "wjbtools");
        QVERIFY(pkg_sys_apps_ww_wjbtools >= 0);

        // not installed, two versions: 39.0-r2 and ~9999
        pkg_app_accessibility_ww_emacspeak =
            findPkg(cat_app_accessibility, "emacspeak");
        QVERIFY(pkg_app_accessibility_ww_emacspeak >= 0);

        // not installed, one version: ~9999
        pkg_app_accessibility_ww_simon =
            findPkg(cat_app_accessibility, "simon");
        QVERIFY(pkg_app_accessibility_ww_simon >= 0);
    }
}

void TestReportModelItem::cleanupTestCase()
{
}

void TestReportModelItem::cleanup()
{
}

void TestReportModelItem::test_name()
{
    const eix_proto::Category &cat = eix.category(cat_dev_qt);
    const eix_proto::Package &pkg = cat.package(pkg_dev_qt_ww_qt_creator);
    PackageReportItem something(cat.category(), pkg, emptyVersionMap);

    QCOMPARE(something.category(), "dev-qt");
    QCOMPARE(something.name(), "qt-creator");
}

void TestReportModelItem::test_description()
{
    const eix_proto::Category &cat = eix.category(cat_dev_qt);
    const eix_proto::Package &pkg = cat.package(pkg_dev_qt_ww_qt_creator);
    PackageReportItem something(cat.category(), pkg, emptyVersionMap);

    QCOMPARE(something.description(),
             cat.package(pkg_dev_qt_ww_qt_creator).description());
}

void TestReportModelItem::test_data_member()
{
    const eix_proto::Category &cat = eix.category(cat_dev_qt);
    const eix_proto::Package &pkg = cat.package(pkg_dev_qt_ww_qt_creator);
    PackageReportItem something(cat.category(), pkg, emptyVersionMap);

    QVariant installed = something.data(PackageReportItem::Column::Installed,
                                        Qt::DecorationRole);
    QCOMPARE(installed.typeName(), "QIcon");
    // It would be nice to compare the icon with the expected one. However,
    // QIcon does not know the resource path and QIcon has no compare operator.
    // Could convert the reference image and this one to QImage and compare
    // size, format, and every pixel. Maybe someday ...

    QCOMPARE(something.data(PackageReportItem::Column::Name, Qt::DisplayRole)
                 .toString()
                 .toStdString(),
             pkg.name());

    QCOMPARE(
        something
            .data(PackageReportItem::Column::InstalledVersion, Qt::DisplayRole)
            .toString(),
        QStringLiteral("(~)4.12.3"));

    QCOMPARE(
        something
            .data(PackageReportItem::Column::AvailableVersion, Qt::DisplayRole)
            .toString(),
        QStringLiteral("~4.12.3"));

    QCOMPARE(something.data(PackageReportItem::Description, Qt::DisplayRole)
                 .toString()
                 .toStdString(),
             pkg.description());
}

void TestReportModelItem::test_data_member_with_999()
{
    const eix_proto::Category &cat = eix.category(cat_app_accessibility);
    const eix_proto::Package &pkg =
        cat.package(pkg_app_accessibility_ww_emacspeak);
    PackageReportItem something(cat.category(), pkg, emptyVersionMap);

    QVariant installed = something.data(PackageReportItem::Column::Installed,
                                        Qt::DecorationRole);
    QCOMPARE(installed, QVariant());

    QCOMPARE(something.data(PackageReportItem::Column::Name, Qt::DisplayRole)
                 .toString()
                 .toStdString(),
             pkg.name());

    QCOMPARE(
        something
            .data(PackageReportItem::Column::InstalledVersion, Qt::DisplayRole)
            .toString(),
        QStringLiteral(""));

    QCOMPARE(
        something
            .data(PackageReportItem::Column::AvailableVersion, Qt::DisplayRole)
            .toString(),
        QStringLiteral("39.0-r2"));

    QCOMPARE(something.data(PackageReportItem::Description, Qt::DisplayRole)
                 .toString()
                 .toStdString(),
             pkg.description());
}

void TestReportModelItem::test_data_member_only_999()
{
    const eix_proto::Category &cat = eix.category(cat_app_accessibility);
    const eix_proto::Package &pkg = cat.package(pkg_app_accessibility_ww_simon);
    PackageReportItem something(cat.category(), pkg, emptyVersionMap);

    QVariant installed = something.data(PackageReportItem::Column::Installed,
                                        Qt::DecorationRole);
    QCOMPARE(installed, QVariant());

    QCOMPARE(something.data(PackageReportItem::Column::Name, Qt::DisplayRole)
                 .toString()
                 .toStdString(),
             pkg.name());

    QCOMPARE(
        something
            .data(PackageReportItem::Column::InstalledVersion, Qt::DisplayRole)
            .toString(),
        QStringLiteral(""));

    QCOMPARE(
        something
            .data(PackageReportItem::Column::AvailableVersion, Qt::DisplayRole)
            .toString(),
        QStringLiteral("~9999"));

    QCOMPARE(something.data(PackageReportItem::Description, Qt::DisplayRole)
                 .toString()
                 .toStdString(),
             pkg.description());
}

void TestReportModelItem::test_copy_constructor()
{
    const eix_proto::Category &cat = eix.category(cat_dev_qt);
    const eix_proto::Package &pkg1 = cat.package(pkg_dev_qt_ww_qt_creator);
    PackageReportItem first(cat.category(), pkg1, emptyVersionMap);

    PackageReportItem second(first);
    QCOMPARE(second.name(), first.name());
    QCOMPARE(second.category(), first.category());
    QCOMPARE(second.description(), first.description());
    QCOMPARE(second.installType(), first.installType());
    QCOMPARE(second.installed(), first.installed());
    QCOMPARE(second.versionNames(), first.versionNames());

    // TODO: check the data() also
}

void TestReportModelItem::test_vector_of()
{
    const eix_proto::Category &cat = eix.category(cat_dev_qt);
    const eix_proto::Package &pkg1 = cat.package(pkg_dev_qt_ww_qt_creator);
    PackageReportItem first(cat.category(), pkg1, emptyVersionMap);

    const eix_proto::Package &pkg2 = cat.package(pkg_dev_qt_ww_qtcore);
    PackageReportItem second(cat.category(), pkg2, emptyVersionMap);

    QVector<PackageReportItem> vector;
    vector.append(first);
    vector.append(second);

    QCOMPARE(vector[0].name(), "qt-creator");
    QCOMPARE(vector[1].name(), "qtcore");
}

void TestReportModelItem::test_cached_values()
{
    const eix_proto::Category &cat1 = eix.category(cat_dev_qt);
    const eix_proto::Category &cat2 = eix.category(cat_sys_apps);

    // TODO: check versions

    PackageReportItem qtcreator(cat1.category(),
                                cat1.package(pkg_dev_qt_ww_qt_creator),
                                emptyVersionMap);
    QVERIFY(qtcreator.installed());
    QVERIFY(qtcreator.installType());

    PackageReportItem qtcore(cat1.category(),
                             cat1.package(pkg_dev_qt_ww_qtcore),
                             emptyVersionMap);
    QVERIFY(qtcore.installed());
    QVERIFY(!qtcore.installType());

    PackageReportItem qtdiag(cat1.category(),
                             cat1.package(pkg_dev_qt_ww_qtdiag),
                             emptyVersionMap);
    QVERIFY(!qtdiag.installed());
    QVERIFY(!qtdiag.installType());

    PackageReportItem wjbtools(cat2.category(),
                               cat2.package(pkg_sys_apps_ww_wjbtools),
                               emptyVersionMap);
    QVERIFY(wjbtools.installed());
    QVERIFY(wjbtools.installType());
}

int TestReportModelItem::findCat(std::string catName)
{
    for (int catNumber = 0; catNumber < eix.category_size(); ++catNumber) {
        if (catName == eix.category(catNumber).category()) {
            return catNumber;
        }
    }
    return -1;
}

int TestReportModelItem::findPkg(int catNumber, std::string pkgName)
{
    if (catNumber >= 0 && catNumber < eix.category_size()) {
        const eix_proto::Category &cat = eix.category(catNumber);
        for (int pkgNumber = 0; pkgNumber < cat.package_size(); ++pkgNumber) {
            if (pkgName == cat.package(pkgNumber).name()) {
                return pkgNumber;
            }
        }
    }
    return -1;
}

QTEST_APPLESS_MAIN(TestReportModelItem)

#include "testpackagereportitem.moc"
