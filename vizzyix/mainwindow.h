// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QActionGroup>
#include <QDateTime>
#include <QItemSelection>
#include <QLineEdit>
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QString>

#include "HTML.h"
#include "applicationdata.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private:
    Ui::MainWindow *ui;
    void log(QString message);
    void aboutVizzyix();
    void adjustCategoryTreeColumns();
    void adjustPackageTableColumns();
    void setupCategoryTreeModelData();
    void fixupLineClearButton(QLineEdit *lineEdit);

    void showEbuildSource(const QUrl &url, const QString &label);
    void showPackageDetails(const PackageReportItem &item);

    bool isDataConsistent();

    ApplicationData applicationData;

    QSortFilterProxyModel packageProxyModel;
    QTemporaryFile ebuildContent;
    HTML::Document *html = new HTML::Document();

    QActionGroup *selectionStateGroup = nullptr;
    QLineEdit *searchBox = nullptr;

    bool firstTimeShown = true;

  signals:
    void loadPortageData();

  private slots:
    void onReady();
    void onEixRunning(bool running);
    void onCategorySelected(const QItemSelection &selected,
                            const QItemSelection &deselected);
    void onPackageSelected(const QItemSelection &selected,
                           const QItemSelection &deselected);

    void onSelectAll();
    void onSelectInstalled();
    void onSelectWorld();
    void onSearchText();

    void onpackageDescriptionLinkClicked(const QUrl &url);

    void aboutQt();
};
