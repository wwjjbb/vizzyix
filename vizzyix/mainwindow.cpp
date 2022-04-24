// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "mainwindow.h"

#include <QDateTime>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QTextBrowser>
#include <QTimer>
#include <iostream>

#include "HTML.h"
#include "aboutdialog.h"
#include "customhtmlelements.h"
#include "eix.pb.h"
#include "eixprotohelper.h"
#include "searchboxvalidator.h"
#include "ui_mainwindow.h"

/*
 * MainWindowqDebug
 *
 * The MainWindow class provides the user interface for Vizzyix.
 */

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->packageListView->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->packageListView->horizontalHeader()->setMinimumSectionSize(20);

    // Menu

    connect(ui->actionAbout, &QAction::triggered, this,
            &MainWindow::aboutVizzyix);
    connect(ui->actionAbout_Qt, &QAction::triggered, this,
            &MainWindow::aboutQt);
    connect(ui->actionReload, &QAction::triggered, this,
            &MainWindow::loadPortageData);
    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);

    // Toolbar - main filters

    selectionStateGroup = new QActionGroup(this);
    selectionStateGroup->addAction(ui->actionSelectAll);
    selectionStateGroup->addAction(ui->actionSelectInstalled);
    selectionStateGroup->addAction(ui->actionSelectWorld);
    selectionStateGroup->setExclusive(true);

    connect(ui->actionSelectAll, &QAction::triggered, this,
            &MainWindow::onSelectAll);
    connect(ui->actionSelectInstalled, &QAction::triggered, this,
            &MainWindow::onSelectInstalled);
    connect(ui->actionSelectWorld, &QAction::triggered, this,
            &MainWindow::onSelectWorld);

    // Database & models

    connect(this, &MainWindow::loadPortageData, &applicationData,
            &ApplicationData::loadPortageData);

    connect(&applicationData, &ApplicationData::categoryModelUpdated, this,
            &MainWindow::setupCategoryTreeModelData);

    // Data
    connect(&applicationData, &ApplicationData::eixRunning, this,
            &MainWindow::onEixRunning);

    // Toolbar - text search

    searchBox = new QLineEdit(this);
    searchBox->setClearButtonEnabled(true);
    connect(searchBox, &QLineEdit::returnPressed, this,
            &MainWindow::onSearchText);
    fixupLineClearButton(searchBox);
    searchBox->setValidator(new SearchBoxValidator(this));

    QLabel *searchLabel = new QLabel(" Search: ");

    ui->toolBar->addSeparator();
    ui->toolBar->addWidget(searchLabel);
    ui->toolBar->addWidget(searchBox);

    // Connect up the packageDescription signals

    connect(ui->packageDescription,
            QOverload<const QUrl &>::of(&QTextBrowser::anchorClicked), this,
            &MainWindow::onpackageDescriptionLinkClicked);

    // Assign all the models, they have all been constructed complete/empty

    ui->categoryTree->setModel(&applicationData.categoryTreeModel);
    packageProxyModel.setSourceModel(&applicationData.packageModel);
    ui->packageListView->setModel(&packageProxyModel);

    QFont boldFont(ui->packageListView->font());
    boldFont.setWeight(QFont::Bold);
    PackageReportItem::setBoldFont(boldFont);

    // Arrange for startup event
    QTimer::singleShot(0, this, SLOT(onReady()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::log(QString message)
{
    ui->packageDescription->append(message);
}

void MainWindow::aboutVizzyix()
{
    AboutDialog about(this);

    about.exec();
}

void MainWindow::adjustCategoryTreeColumns()
{
    const int pkgsWidth = 80;

    // Set the width of the first column so that the second column has just
    // enough space to show its header, and 4 digit numbers. Base on the current
    // size of the control.
    int residualWidth = ui->categoryTree->width() - pkgsWidth;
    ui->categoryTree->setColumnWidth(0, residualWidth);
}

void MainWindow::adjustPackageTableColumns()
{
    ui->packageListView->setColumnWidth(PackageReportItem::Column::Installed,
                                        24);
    ui->packageListView->setColumnWidth(PackageReportItem::Column::Name, 170);
}

/*
 * MainWindow::setupCategoryModelData
 *
 * Displays the category tree when the model has been changed.
 */
void MainWindow::setupCategoryTreeModelData()
{
    ui->categoryTree->hideColumn(CategoryTreeItem::Column::CatIndex);
    auto allNode = ui->categoryTree->model()->index(0, 0);
    ui->categoryTree->setExpanded(allNode, true);

    // Don't want this armed till some data is there. The final flag,
    // UniqueConnection, means there will only be one connection no matter
    // how many times this bit runs...
    connect(ui->categoryTree->selectionModel(),
            QOverload<const QItemSelection &, const QItemSelection &>::of(
                &QItemSelectionModel::selectionChanged),
            this, &MainWindow::onCategorySelected, Qt::UniqueConnection);

    ui->categoryTree->setCurrentIndex(allNode);

    adjustCategoryTreeColumns();
}

/*
 * MainWindow::setEixRunning
 *
 * Disables some form controls when eix is running
 * - turn off the Form|Reload option
 * - prevent changes to search text
 */
void MainWindow::onEixRunning(bool running)
{
    ui->actionReload->setEnabled(!running);
    if (searchBox != nullptr) {
        searchBox->setEnabled(!running);
    }
}

/*
 * MainWindow::fixupLineClearButton
 *
 * Add event to the LineEdit to react to 'clear' being pressed
 *
 * There is no event for when the line edit's clear button is pressed, so
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
            connect(myClearAction, &QAction::triggered, lineEdit,
                    &QLineEdit::returnPressed, Qt::QueuedConnection);
        }
    }
}

/*
 * MainWindow::showEbuildSource
 *
 * Shows the source of the specified ebuild, using a monospaced font. The
 * listing is prefixed with a header area:
 *   - the title is the specified label value, which should the ebuild filename
 *   - a clickable back button to return to the list of package versions.
 *
 * url:
 *     The url for the ebuild
 * label:
 *     THe title for the listing
 */
void MainWindow::showEbuildSource(const QUrl &url, const QString &label)
{
    QFile ebuildFile(url.path());
    if (ebuildFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream text(&ebuildFile);
        QString ebuildText = text.readAll();
        ebuildFile.close();

        if (ebuildContent.open()) {
            QTextStream htmlStream(&ebuildContent);
            htmlStream << "<h1>" << label << "</h1>";
            htmlStream << "<a href=BACK>Back</a>";
            htmlStream << "<hr>";
            htmlStream << "<pre>";
            htmlStream << ebuildText;
            htmlStream << "</pre>";

            ebuildContent.close();
            ui->packageDescription->setSource(ebuildContent.fileName());
        }
    }
}

void MainWindow::showPackageDetails(const PackageReportItem &item)
{
    if (html != nullptr)
        delete html;

    html = new HTML::Document("Package Report");

    *html << HTML::Header1((item.category() + "/" + item.name()).c_str());

    *html << HTML::Paragraph(item.description().c_str());

    // There can be more than one homepage listed. If so, the names are
    // separated by a space.
    *html << HTML::Paragraph("Home: ");
    auto homepage =
        QString(item.packageDetails().homepage().c_str()).split(" ");
    for (int hp = 0; hp < homepage.length(); ++hp) {
        *html << HTML::Link(homepage[hp].toStdString(),
                            homepage[hp].toStdString());
    }

    QString licenses =
        QString(item.packageDetails().licenses().c_str()).replace(" ", ", ");
    if (!licenses.isEmpty()) {
        *html << HTML::Paragraph("License: " + licenses.toStdString());
    }

    // TODO: simplify the per-version report
    //   * For installed apps, show the icon (as used in the main index)
    //   * For keyworded apps, enclose the "~" in brackets
    //   * show the slot number (if there is one)
    //   * identify masked versions with [m]

    // Now show details for each version
    for (int vn = 0; vn < item.packageDetails().version_size(); ++vn) {
        const auto &version = item.packageDetails().version(vn);

        *html << HTML::Hr();

        // Check whether this version is stable (on the local arch)
        bool stable = EixProtoHelper::isStable(version);

        // Bracketted repository (overlay) name. Empty string if repo is
        // "gentoo"
        QString repoNameDisplay = "";
        QString repoName = "";

        if (version.has_repository()) {
            repoName = version.repository().repository().c_str();
            if (!repoName.isEmpty()) {
                repoNameDisplay = QString("(%1)").arg(repoName);
            }
        }

        *html << HTML::Header2(
            (QString::fromStdString((stable ? "" : "(~)") + version.id()) +
             repoNameDisplay)
                .toStdString());

        // Say if version is installed or not
        if (version.has_installed()) {
            std::time_t dateInstalled = version.installed().date();
            QDateTime date(QDateTime::fromTime_t(dateInstalled));

            auto installType = EixProtoHelper::classifyInstallType(version);
            QString installIcon;

            switch (installType) {
            case eix_proto::MaskFlags_MaskFlag_WORLD:
                installIcon = "qrc:/pkgtable/images/installed-world-marker.png";
                break;
            case eix_proto::MaskFlags_MaskFlag_WORLD_SETS:
                installIcon = "qrc:/pkgtable/images/installed-set-marker.png";
                break;
            case eix_proto::MaskFlags_MaskFlag_MASK_SYSTEM:
                installIcon =
                    "qrc:/pkgtable/images/installed-system-marker.png";
                break;
            default:
                installIcon = "qrc:/pkgtable/images/installed-marker.png";
                break;
            }

            *html << HTML::Image(installIcon.toStdString(),
                                 installIcon.toStdString(), 24, 24);
            *html << HTML::Paragraph(
                ("Installed: " + date.toString(Qt::ISODate) + " ")
                    .toStdString());
            // TODO: improve layout
        }

        QString useflags = EixProtoHelper::useFlagSummary(version);
        if (!useflags.isEmpty())
            *html << HTML::Paragraph(
                QString("USE: %1").arg(useflags).toStdString());

        QString repoDir = applicationData.repositoryIndex.find(repoName);
        if (repoDir.isEmpty()) {
            *html << HTML::Paragraph(
                QString("EBUILD: can't find directory for %1 in repos.conf")
                    .arg(repoName)
                    .toStdString());
        } else {
            // (I wish QString would get on better with std::string)
            auto ebuildFile =
                QString("%1-%2.ebuild")
                    .arg(item.name().c_str(), version.id().c_str());
            auto ebuildPath =
                QString("%1/%2/%3/%4")
                    .arg(applicationData.repositoryIndex.find(repoName),
                         item.category().c_str(), item.name().c_str(),
                         ebuildFile);
            *html << HTML::Paragraph("EBUILD: ")
                  << HTML::Link(ebuildFile.toStdString(),
                                ebuildPath.toStdString());
        }

        // Eix does not provide much more.
        //   * the use available and installed-with flags, maybe.
        //   * the restrictions
        //   * the properties
        //   * installation date, EAPI, and use flags enabled/disabled
    }

    *html << HTML::Hr();

    auto zoms = applicationData.packageList.zombies();
    if (zoms.length() != 0) {
        *html << HTML::Paragraph(
            QString("Zombies: %1")
                .arg(applicationData.packageList.zombies().join(", "))
                .toStdString());
    }
    ui->packageDescription->setHtml(QString::fromStdString(html->toString()));
}

/*
 * MainWindow::checkDatabaseAges
 *
 * Checks whether the database files and loaded database are consistent.
 * It does this by comparing the various data file dates. It may report,
 * via a dialog, that eix-update needs to be run - can't do this by itself.
 *
 * Returns false if the portage data needs to be reloaded from scratch.
 */
bool MainWindow::isDataConsistent()
{
    bool result = true;

    // TODO: all the filenames should be properties

    // The date of emerge log file is used to determine the time of the last
    // install.
    QFileInfo emergeLogFile("/var/log/emerge.log");

    // The date of the portage.eix file is used to determine the last time
    // eix-update was run
    QFileInfo portageEixFile("/var/cache/eix/portage.eix");

    // The date of the timestamp.chk file is used to determine the last gentoo
    // repo sync time.
    // TODO: determine by reading the repository list
    QFileInfo syncTimestampFile("/var/db/repos/gentoo/metadata/timestamp.chk");

    if (emergeLogFile.exists() && portageEixFile.exists() &&
        syncTimestampFile.exists()) {
        QDateTime lastEmerge = emergeLogFile.lastModified();
        QDateTime lastEixUpdate = portageEixFile.lastModified();
        QDateTime lastSync = syncTimestampFile.lastModified();

        if (lastEmerge > lastEixUpdate || lastSync > lastEixUpdate) {
            // Since the previous eix-update, there's been a sync or emerge

            QMessageBox::warning(this, QString("Eix database is out of date"),
                                 QString("Please run \"eix-update\" in a "
                                         "console to get the latest data\n"
                                         "and then select menu option \"File | "
                                         "Reload EIX Database\"\n"
                                         "to show the new data.\n"
                                         "\n"
                                         ""));
        } else if (applicationData.lastLoadTime < lastEixUpdate) {
            qDebug() << "Reload the package data";
            result = false;
        } else {
            qDebug() << "Information: everything appears to be up-to-date";
        }
    } else {
        if (!emergeLogFile.exists()) {
            qDebug() << "Warning: " << emergeLogFile.absoluteFilePath()
                     << "does not exist";
        }
        if (!portageEixFile.exists()) {
            qDebug() << "Warning: " << portageEixFile.absoluteFilePath()
                     << "does not exist";
        }
        if (!syncTimestampFile.exists()) {
            qDebug() << "Warning: " << syncTimestampFile.absoluteFilePath()
                     << "does not exist";
        }
    }

    return result;
}

/*
 * MainWindow::onReady
 *
 * This should happen straight after the creation of the objects, via a
 * singleshot timer set in the constructor.
 */
void MainWindow::onReady()
{
    // It should not take long to read a few small files, and it needs to be
    // done before the other data is loaded
    applicationData.repositoryIndex.load();

    emit loadPortageData();

    (void)isDataConsistent();
}

void MainWindow::onCategorySelected(const QItemSelection &selected,
                                    const QItemSelection &)
{
    const QModelIndexList &list = selected.indexes();
    if (list.length() == 1) {

        packageProxyModel.setSortCaseSensitivity(Qt::CaseInsensitive);

        // This is from the demo implementation of a tree model. Just as well
        // really, because it would have taken a long time to figure it out from
        // the docs.
        CategoryTreeItem *item =
            static_cast<CategoryTreeItem *>(list[0].internalPointer());

        applicationData.setupPackageModelData(item);

        packageProxyModel.sort(PackageReportItem::Column::Name);

        // Don't really want this armed till something is there. The final flag,
        // UniqueConnection, means there will only be one connection no matter
        // how many times this bit runs...
        connect(ui->packageListView->selectionModel(),
                QOverload<const QItemSelection &, const QItemSelection &>::of(
                    &QItemSelectionModel::selectionChanged),
                this, &MainWindow::onPackageSelected, Qt::UniqueConnection);

        ui->packageListView->setCurrentIndex(packageProxyModel.index(0, 0));
        adjustPackageTableColumns();

    } else {
        qDebug() << list.length() << "items selected - should not happen";
    }
}

void MainWindow::onPackageSelected(const QItemSelection &selected,
                                   const QItemSelection &)
{
    const QModelIndexList &list = selected.indexes();
    if (list.length() > 0) {

        // The QModelIndex is for the sort proxy, so it needs to be translated
        // to an index for the base package model.
        const PackageReportItem &item =
            applicationData.packageModel.packageItem(
                packageProxyModel.mapToSource(list[0]).row());

        showPackageDetails(item);

    } else {
        qDebug() << list.length() << "items selected - should not happen";
    }
}

void MainWindow::onSelectAll()
{
    applicationData.setSelectionFilter(ApplicationData::SelectionFilter::All);
    applicationData.runEix();
}

void MainWindow::onSelectInstalled()
{
    applicationData.setSelectionFilter(
        ApplicationData::SelectionFilter::Installed);
    applicationData.runEix();
}

void MainWindow::onSelectWorld()
{
    applicationData.setSelectionFilter(ApplicationData::SelectionFilter::World);
    applicationData.runEix();
}

void MainWindow::onSearchText()
{
    applicationData.setSearch(searchBox->text());
    applicationData.runEix();
}

void MainWindow::onpackageDescriptionLinkClicked(const QUrl &url)
{
    QString filetype = QFileInfo(url.fileName()).suffix();
    if (filetype == "ebuild") {
        QString filename = QFileInfo(url.fileName()).fileName();
        showEbuildSource(url, filename);
    }
    if (url.path() == "BACK" && html != nullptr) {
        ui->packageDescription->setHtml(
            QString::fromStdString(html->toString()));
    }
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, "Vizzyix");
}
