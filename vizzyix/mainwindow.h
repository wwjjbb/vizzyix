// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QActionGroup>
#include <QDateTime>
#include <QItemSelection>
#include <QLineEdit>
#include <QMainWindow>
#include <QProcess>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTemporaryFile>

#include "HTML.h"
#include "categorytreemodel.h"
#include "combinedpackagelist.h"
#include "eix.pb.h"
#include "packagereportmodel.h"
#include "repositoryindex.h"

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
    enum SelectionState { All, Installed, World };

    Ui::MainWindow *ui;
    void log(QString message);
    void aboutVizzyix();
    void cleanupEixProcess();
    void adjustCategoryTreeColumns();
    void addCategory(CategoryTreeItem *catItem);
    void adjustPackageTableColumns();
    void setSelectionState(SelectionState state);
    void setupCategoryModelData();
    void setEixRunning(bool running);
    void fixupLineClearButton(QLineEdit *lineEdit);

    void showEbuildSource(const QUrl &url, const QString &label);
    void showPackageDetails(const PackageReportItem &item);

    void checkDatabaseAges(bool enableReload);

    SelectionState selectionState() const;

    QProcess *eixProcess = nullptr;

    eix_proto::Collection eix;
    CategoryTreeModel categoryModel;
    PackageReportModel packageModel;
    CombinedPackageList packageList;
    RepositoryIndex repositoryIndex;
    QSortFilterProxyModel packageProxyModel;
    QTemporaryFile *eixOutput = nullptr;
    QTemporaryFile ebuildContent;
    HTML::Document *html = new HTML::Document();
    QDateTime lastLoadTime;

    SelectionState selectionState_ = SelectionState::All;

    QActionGroup *selectionStateGroup = nullptr;
    QLineEdit *searchBox = nullptr;

    bool firstTimeShown = true;

  protected:
    void showEvent(QShowEvent *event);

  signals:
    void reloadPackageData();

  private slots:
    void checkStatusAndReload();
    void runEix();
    void parseEixData();
    void onEixFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onEixError(QProcess::ProcessError error);
    void onCategorySelected(const QItemSelection &selected,
                            const QItemSelection &deselected);
    void onPackageSelected(const QItemSelection &selected,
                           const QItemSelection &deselected);

    void onShow();

    void onSelectAll();
    void onSelectInstalled();
    void onSelectWorld();
    void onSearchText();

    void onpackageDescriptionLinkClicked(const QUrl &url);

    void aboutQt();
};
#endif // MAINWINDOW_H
