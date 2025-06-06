// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "mainwindow.h"

#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QStandardItemModel>
#include <QTextBrowser>
#include <QTimer>
#include <QtLogging>

#include "aboutdialog.h"
#include "eix.pb.h"
#include "eixprotohelper.h"
#include "searchboxvalidator.h"
#include "ui_mainwindow.h"

/*!
 * Attaches to the Designer gui data, wires up the signals/slots, and
 * sets (empty) data models for the tree and table views.
 * Also adjusts widget properties where it is not possible to use
 * Designer to get the required result.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->packageListView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->packageListView->horizontalHeader()->setMinimumSectionSize(20);

    // Menu

    connect(ui->actionAbout,
            &QAction::triggered,
            this,
            &MainWindow::aboutVizzyix);
    connect(ui->actionAbout_Qt,
            &QAction::triggered,
            this,
            &MainWindow::aboutQt);
    connect(ui->actionReload,
            &QAction::triggered,
            this,
            &MainWindow::loadPortageData);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);

    // Toolbar - main filters

    // QActionGroup added at runtime to group the SelectXXX actions so
    // that only one at a time can be selected. Unfortunately, it's not
    // possible to set this up in Designer (in 2024, anyway).
    // No need to explicitly delete since it is owned by this window.
    auto selectionStateGroup = new QActionGroup(this);
    selectionStateGroup->addAction(ui->actionSelectAll);
    selectionStateGroup->addAction(ui->actionSelectInstalled);
    selectionStateGroup->addAction(ui->actionSelectWorld);
    selectionStateGroup->setExclusive(true);

    connect(ui->actionSelectAll,
            &QAction::triggered,
            this,
            &MainWindow::onSelectAll);
    connect(ui->actionSelectInstalled,
            &QAction::triggered,
            this,
            &MainWindow::onSelectInstalled);
    connect(ui->actionSelectWorld,
            &QAction::triggered,
            this,
            &MainWindow::onSelectWorld);

    // Database & models

    connect(this,
            &MainWindow::loadPortageData,
            ApplicationData::data(),
            &ApplicationData::loadPortageData);

    connect(ApplicationData::data(),
            &ApplicationData::categoryModelUpdated,
            this,
            &MainWindow::displayCategoryTree);

    // Data
    connect(ApplicationData::data(),
            &ApplicationData::eixRunning,
            this,
            &MainWindow::onEixRunning);

    // Toolbar - text search

    _searchBox = new QLineEdit(this);
    _searchBox->setClearButtonEnabled(true);
    connect(_searchBox,
            &QLineEdit::returnPressed,
            this,
            &MainWindow::onSearchText);
    fixupLineClearButton(_searchBox);
    _searchBox->setValidator(new SearchBoxValidator(this));

    QLabel *searchLabel = new QLabel(" Search: ");

    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(searchLabel);
    ui->toolBar->addWidget(_searchBox);

    // Assign all the models, they have all been constructed complete/empty

    ui->categoryTree->setModel(&ApplicationData::data()->categoryTreeModel);
    _packageProxyModel.setSourceModel(
        &ApplicationData::data()->packageReportModel);
    ui->packageListView->setModel(&_packageProxyModel);

    QFont boldFont(ui->packageListView->font());
    boldFont.setWeight(QFont::Bold);
    PackageReportItem::setBoldFont(boldFont);

    _detailsDialog = new DetailsDialog(this);
    connect(this,
            &MainWindow::showEbuild,
            _detailsDialog,
            &DetailsDialog::showEbuild);
    connect(ui->ebuildList,
            &QTableView::clicked,
            this,
            &MainWindow::onClickedVersion);

    // Arrange for startup outside of the constructor
    emit loadPortageData();
}

/// Only has to free up the ui object
MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * Set the width of Categories column in tree view so it's as wide as possible
 * while leaving space for a 5-digit package count in the Pkgs column.
 */
void MainWindow::adjustCategoryTreeColumns()
{
    // It's probably better to check for N digits experimentally rather
    // than guess pixels, I suppose. I do wish for a way to say, "I want
    // to fit this text string into a column please, how wide??".
    // Using a number because I want the columns to stay same width
    // regardless.
    auto fontMetrics = ui->packageListView->fontMetrics();
    auto size = fontMetrics.size(Qt::TextSingleLine, "1234567890");

    int categoryWidth = ui->categoryTree->width() - size.width();
    ui->categoryTree->setColumnWidth(0, categoryWidth);
}

/*!
 * Adjust the icon colummn and package name column sizes of the category tree
 * view to fit content. It's a bit arbitrary, but whatever.
 */
void MainWindow::adjustPackageTableColumns()
{
    ui->packageListView->setColumnWidth(PackageReportItem::Column::Installed,
                                        24);
    ui->packageListView->setColumnWidth(PackageReportItem::Column::Name, 170);
}

/*!
 * Displays the category tree when the model has been changed.
 */
void MainWindow::displayCategoryTree()
{
    ui->categoryTree->hideColumn(CategoryTreeItem::Column::CatIndex);

    // Expand all the nodes
    auto allNode = ui->categoryTree->model()->index(0, 0);
    ui->categoryTree->setExpanded(allNode, true);

    // Don't want this armed till some data is there. The final flag,
    // UniqueConnection, means there will only be one connection no matter
    // how many times this bit runs...
    connect(ui->categoryTree->selectionModel(),
            QOverload<const QItemSelection &, const QItemSelection &>::of(
                &QItemSelectionModel::selectionChanged),
            this,
            &MainWindow::onCategorySelected,
            Qt::UniqueConnection);

    ui->categoryTree->setCurrentIndex(allNode);

    adjustCategoryTreeColumns();
}
/*!
 * Add event to a QLineEdit widget to react to 'clear' being pressed
 *
 * There is no event for when a QLineEdit's clear button is pressed, so
 * by default the application does not know this has happened until the
 * user hits RETURN.
 *
 * The following hack locates the clear button and adds an action that
 * makes pressing the button signal "QLineEdit::returnPressed". See
 * relevant Qt bug,
 *     https://bugreports.qt.io/browse/QTBUG-36257
 *
 * lineEdit:
 *     The LineEdit widget to be patched
 */
void MainWindow::fixupLineClearButton(QLineEdit *lineEdit)
{
    for (int i(0); i < lineEdit->children().size(); ++i) {
        QAction *myClearAction(
            qobject_cast<QAction *>(lineEdit->children().at(i)));
        if (myClearAction) {
            connect(myClearAction,
                    &QAction::triggered,
                    lineEdit,
                    &QLineEdit::returnPressed,
                    Qt::QueuedConnection);
        }
    }
}

/*!
 * Shows the package information and lists the available versions.
 */
void MainWindow::showPackageDetails(const PackageReportItem &item)
{
    // TODO - Show slot numbers

    _htmlDescription.clear();

    _htmlDescription.header(1,
                            QStringLiteral("%1/%2")
                                .arg(item.category().c_str())
                                .arg(item.name().c_str()));
    _htmlDescription.para(item.description().c_str());

    // There can be more than one homepage listed. If so, the names are
    // separated by a space.
    auto homepage =
        QString::fromStdString(item.packageDetails().homepage()).split(" ");
    for (int hp = 0; hp < homepage.length(); ++hp) {
        // TODO - simplify the displayed URL
        // "Sourceforge", "github", <just the domain>
        _htmlDescription.link(homepage[hp], homepage[hp]);
    }

    QString licenses = QString::fromStdString(item.packageDetails().licenses())
                           .replace(" ", ", ");
    if (!licenses.isEmpty()) {
        // TODO - link the license to the file in portage d/b
        // Open where though?
        _htmlDescription.para("License: " + licenses);
    }

    // TODO: simplify the per-version report
    //   * For installed apps, show the icon (as used in the main index)
    //   * For keyworded apps, enclose the "~" in brackets
    //   * show the slot number (if there is one)
    //   * identify masked versions with [m]

    _detailsModel.clear();
    _detailsModel.setRowCount(item.packageDetails().version_size());
    _detailsModel.setColumnCount(EbuildColumn::ColumnCount);

    _detailsModel.setHorizontalHeaderLabels(
        {"Version", "Repository", "Date", "Reason"});
    // Now show details for each version
    for (int vn = 0; vn < item.packageDetails().version_size(); ++vn) {
        const auto &version = item.packageDetails().version(vn);

        // Check whether this version is stable (on the local arch)
        bool stable = EixProtoHelper::isStable(version);

        // Empty string if repo is "gentoo"
        QString repoName{""}; // "<reponame>"
        if (version.has_repository()) {
            repoName = version.repository().repository().c_str();
        }

        // TODO -also have a colour to highlight lines
        // Empty string if stable, "(~)" for installed testing, "~" for testing
        QString stableMarker{""};
        if (!stable) {
            stableMarker = version.has_installed() ? "(~)" : "~";
        }

        _detailsModel.setItem(vn,
                              EbuildColumn::Category,
                              new QStandardItem(item.category().c_str()));
        _detailsModel.setItem(vn,
                              EbuildColumn::Package,
                              new QStandardItem(item.name().c_str()));
        _detailsModel.setItem(vn,
                              EbuildColumn::Version,
                              new QStandardItem(version.id().c_str()));

        QString versionDetail = stableMarker + version.id().c_str();

        _detailsModel.setItem(vn,
                              EbuildColumn::VersionDetail,
                              new QStandardItem(versionDetail));
        _detailsModel.setItem(vn,
                              EbuildColumn::Repository,
                              new QStandardItem(repoName));

        // Say if version is installed or not
        if (version.has_installed()) {
            std::time_t dateInstalled = version.installed().date();
            QDateTime date(QDateTime::fromSecsSinceEpoch(dateInstalled));

            auto installType = EixProtoHelper::classifyInstallType(version);
            QString installIcon;
            QString reason;
            switch (installType) {
            case eix_proto::MaskFlags_MaskFlag_WORLD:
                installIcon = "qrc:/pkgtable/images/installed-world-marker.png";
                reason = "World";
                break;
            case eix_proto::MaskFlags_MaskFlag_WORLD_SETS:
                installIcon =
                    "qrc:/pkgtable/images/installed-world-set-marker.png";
                reason = "Set";
                break;
            case eix_proto::MaskFlags_MaskFlag_MASK_SYSTEM:
                installIcon =
                    "qrc:/pkgtable/images/installed-system-marker.png";
                reason = "System";
                break;
            default:
                installIcon = "qrc:/pkgtable/images/installed-marker.png";
                reason = "Dependency";
                break;
            }

            _detailsModel.setItem(
                vn,
                EbuildColumn::Date,
                new QStandardItem(date.toString("yyyy-MM-dd")));
            _detailsModel.setItem(vn,
                                  EbuildColumn::Reason,
                                  new QStandardItem(reason));
        }
    }

    _htmlDescription.hr();

    auto zoms = ApplicationData::data()->combinedPackageList.zombieList();
    if (zoms.length() != 0) {
        _htmlDescription.para(QStringLiteral("Zombies: %1")
                                  .arg(ApplicationData::data()
                                           ->combinedPackageList.zombieList()
                                           .join(", ")));
    }

    _htmlDescription.endHtml();

    ui->packageDescription->setHtml(_htmlDescription.toString());
    ui->ebuildList->setModel(&_detailsModel);
    ui->ebuildList->setColumnHidden(EbuildColumn::Category, true);
    ui->ebuildList->setColumnHidden(EbuildColumn::Package, true);
    ui->ebuildList->setColumnHidden(EbuildColumn::Version, true);
}

/*!
 * Checks whether the database files and loaded database are consistent.
 * It does this by comparing the various data file dates. It may report,
 * via a dialog, that eix-update needs to be run - can't do this by itself.
 *
 * Returns false if the portage data needs to be reloaded from scratch.
 */
bool MainWindow::isDataConsistent()
{
    bool result{true};

    // Date of emerge log file is used to determine time of last install.
    QFileInfo emergeLogFile(ApplicationData::emergeLogFile);

    // Date of portage.eix file is used to determine the last time
    // eix-update was run
    QFileInfo portageEixFile(ApplicationData::portageEixFile);

    // The date of the timestamp.chk file is used to determine the last gentoo
    // repo sync time.
    QString timestampFile =
        QStringLiteral("%1/metadata/timestamp.chk")
            .arg(ApplicationData::data()->findRepositoryPath(
                ApplicationData::defaultRepositoryName));
    QFileInfo syncTimestampFile(timestampFile);

    if (!emergeLogFile.exists()) {
        // There is no emerge.log file!

        // TODO - this should pop up a message and exit
        qCritical() << emergeLogFile.absoluteFilePath() << "does not exist";
        result = false;
    } else if (!syncTimestampFile.exists()) {
        // The Gentoo repository has never been sync'd!

        // TODO - this should pop up a message and exit
        qCritical() << syncTimestampFile.absoluteFilePath() << "does not exist";
        result = false;
    } else if (!portageEixFile.exists()) {
        // The EIX file can be generated by the user running eix-update

        qWarning() << portageEixFile.absoluteFilePath() << "does not exist";

        QMessageBox::warning(
            this,
            QStringLiteral("Eix database does not exist"),
            QStringLiteral("Please run \"eix-update\" in a "
                           "console \n"
                           "and then select menu option \"File | "
                           "Reload EIX Database\"\n"
                           "to show the data.\n"
                           "\n"
                           ""));
        result = false;
    } else {
        // All the necessary files exist to determine when things happened
        // so lets check they happened in the right order.

        QDateTime lastEmerge = emergeLogFile.lastModified();
        QDateTime lastEixUpdate = portageEixFile.lastModified();
        QDateTime lastSync = syncTimestampFile.lastModified();

        if (lastEmerge > lastEixUpdate || lastSync > lastEixUpdate) {

            // Needed the last EIX update to be after last sync or emerge
            // action. It's not, so ask user to run eix-update now.

            // qDebug() << "Last emerge: " << lastEmerge;
            // qDebug() << "Last EIX Update: " << lastEixUpdate;
            qDebug() << "Last Sync: " << lastSync;
            QMessageBox::warning(
                this,
                QStringLiteral("Eix database is out of date"),
                QStringLiteral("Please run \"eix-update\" in a "
                               "console to get the latest data\n"
                               "and then select menu option \"File | "
                               "Reload EIX Database\"\n"
                               "to show the new data.\n"
                               "\n"
                               ""));
            result = false;
        } else if (ApplicationData::data()->lastLoadTime < lastEixUpdate) {

            // Also needed the last data read to be after the last EIX update.
            //
            // This shouldn't be a problem since a search will always
            // do a download, and we're here because of search.

            qWarning() << "A \"can't happen\" just happened!";
            qDebug() << "Last load: " << ApplicationData::data()->lastLoadTime;
            qDebug() << "Last EIX Update: " << lastEixUpdate;
            QMessageBox::warning(this,
                                 QStringLiteral("Eix database has changed"),
                                 QStringLiteral("Please "
                                                "select menu option \"File | "
                                                "Reload EIX Database\"\n"
                                                "to show the new data.\n"
                                                "\n"
                                                ""));
            result = false;
        }
    }

    return result;
}

/// Show the About... dialog
void MainWindow::aboutVizzyix()
{
    AboutDialog about(this);

    about.exec();
}

/*!
 * Disables some form controls while eix is running
 * - turn off the Form|Reload option
 * - prevent changes to search text
 */
void MainWindow::onEixRunning(bool running)
{
    ui->actionReload->setEnabled(!running);
    if (_searchBox != nullptr) {
        _searchBox->setEnabled(!running);
    }

    if (!running) {
        isDataConsistent();
    }
}

/*!
 * When a category has been selected in the category tree.
 */
void MainWindow::onCategorySelected(const QItemSelection &selected,
                                    const QItemSelection &)
{
    const QModelIndexList &list = selected.indexes();
    if (list.length() == 1) {

        _packageProxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);

        // This is from the demo implementation of a tree model. Just as well
        // really, because it would have taken a long time to figure it out from
        // the docs.
        CategoryTreeItem *item =
            static_cast<CategoryTreeItem *>(list[0].internalPointer());

        ApplicationData::data()->setupPackageModelData(item);

        _packageProxyModel.sort(PackageReportItem::Column::Name);

        // Don't really want this armed till something is there. The final flag,
        // UniqueConnection, means there will only be one connection no matter
        // how many times this bit runs...
        connect(ui->packageListView->selectionModel(),
                QOverload<const QItemSelection &, const QItemSelection &>::of(
                    &QItemSelectionModel::selectionChanged),
                this,
                &MainWindow::onPackageSelected,
                Qt::UniqueConnection);

        ui->packageListView->setCurrentIndex(_packageProxyModel.index(0, 0));
        adjustPackageTableColumns();

    } else {
        qCritical() << "onCategorySelected :" << list.length()
                    << "items selected - should not happen";
    }
}

/*!
 * When a package has been selected, show the package details
 */
void MainWindow::onPackageSelected(const QItemSelection &selected,
                                   const QItemSelection &)
{
    const QModelIndexList &list = selected.indexes();
    if (list.length() > 0) {

        // The QModelIndex is for the sort proxy, so it needs to be translated
        // to an index for the base package model.
        const PackageReportItem &item =
            ApplicationData::data()->packageReportModel.packageItem(
                _packageProxyModel.mapToSource(list[0]).row());

        showPackageDetails(item);

    } else {
        qCritical() << "onPackageSelected :" << list.length()
                    << "items selected - should not happen";
    }
}

/// Select all packages to be displayed
void MainWindow::onSelectAll()
{
    ApplicationData::data()->setSelectionFilter(
        ApplicationData::SelectionFilter::All);
    emit loadPortageData();
}

/// Select all installed packages to be displayed
void MainWindow::onSelectInstalled()
{
    ApplicationData::data()->setSelectionFilter(
        ApplicationData::SelectionFilter::Installed);
    emit loadPortageData();
}

/// Select all world packages to be displayed
void MainWindow::onSelectWorld()
{
    ApplicationData::data()->setSelectionFilter(
        ApplicationData::SelectionFilter::World);
    emit loadPortageData();
}

/// Apply a search filter for package names
void MainWindow::onSearchText()
{
    ApplicationData::data()->setSearch(_searchBox->text());
    emit loadPortageData();
}

/*!
 * A version in the package version list has been selected, send
 * the details to the signal to show the version.
 */
void MainWindow::onClickedVersion(const QModelIndex &index)
{
    // Gets the neccessary values from the clicked line (repo may be blank)
    QString repo = index.model()
                       ->data(index.siblingAtColumn(EbuildColumn::Repository))
                       .toString();
    QString category = index.model()
                           ->data(index.siblingAtColumn(EbuildColumn::Category))
                           .toString();
    QString package = index.model()
                          ->data(index.siblingAtColumn(EbuildColumn::Package))
                          .toString();
    QString version = index.model()
                          ->data(index.siblingAtColumn(EbuildColumn::Version))
                          .toString();

    emit showEbuild(repo, category, package, version);
}

/// Show the About Qt Dialog Box.
/// License conditions etc
void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, "Vizzyix");
}
