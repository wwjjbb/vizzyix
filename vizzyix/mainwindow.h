// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QActionGroup>
#include <QDateTime>
#include <QItemSelection>
#include <QLineEdit>
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>

#include "applicationdata.h"
#include "detailsdialog.h"
#include "htmlgenerator.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

/// Provides the user interface for Vizzyix
class MainWindow : public QMainWindow
{
    Q_OBJECT

  private:
    /// Generate column names for the package table view
    enum EbuildColumn {
        VersionDetail,
        Repository,
        Date,
        Reason,
        Category,
        Package,
        Version,
        ColumnCount,
    };

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private:
    void adjustCategoryTreeColumns();
    void adjustPackageTableColumns();
    void displayCategoryTree();
    void fixupLineClearButton(QLineEdit *lineEdit);

    void showPackageDetails(const PackageReportItem &item);

    bool isDataConsistent();

  private:
    /// The gui from designer
    Ui::MainWindow *ui;

    /*!
     * The non-modal dialog used to show details of the selected ebuild.
     * This is created at start and subsequently hidden or shown. The
     * dialog is owned by this window so do not explicitly delete it.
     */
    DetailsDialog *oDetailsDialog;

    /*!
     * Data model for the package list table view. Using a sort filter to
     * make it easy to sort the columms by their values.
     */
    QSortFilterProxyModel oPackageProxyModel;

    /// Builds the content of the summary section
    HtmlGenerator oHtmlDescription;

    /// Data model for the package version table view
    QStandardItemModel oDetailsModel;

    /// Keep a reference to the search filter box. Needed because it gets
    /// accessed throughout.
    QLineEdit *oSearchBox = nullptr;

  signals:
    void loadPortageData();
    void showEbuild(const QString &repository,
                    const QString &category,
                    const QString &package,
                    const QString &version);

  private slots:
    void aboutVizzyix();
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

    void onClickedVersion(const QModelIndex &index);

    void aboutQt();
};
