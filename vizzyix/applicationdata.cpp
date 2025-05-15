// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "applicationdata.h"

#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <QtLogging>
#include <fstream>
#include <iostream>

std::unique_ptr<ApplicationData> ApplicationData::_appData;

/*!
 * Constructor doesn't need to do anything
 */
ApplicationData::ApplicationData()
{
}

/*!
 * Returns a pointer to the single instance of the ApplicationData class.
 * The instance is created if it does not already exist.
 */
ApplicationData *ApplicationData::data()
{
    if (!_appData) {
        _appData.reset(new ApplicationData());
    }
    return _appData.get();
}

/// Whether any filters are currently set
bool ApplicationData::filters()
{
    return (selectionFilter() != SelectionFilter::All) || !search().isEmpty();
}

/// Sets the selection filter to the given value.
void ApplicationData::setSelectionFilter(SelectionFilter filter)
{
    _selectionFilter = filter;
}

/// Gets the current selection filter value.
ApplicationData::SelectionFilter ApplicationData::selectionFilter()
{
    return _selectionFilter;
}

/// Sets the search filter to the given string.
void ApplicationData::setSearch(const QString &search)
{
    _search = search;
}

/// Returns the current search filter.
const QString ApplicationData::search()
{
    return _search;
}

/*!
 * Parses the eix (protobuf format) output into the eix data,
 * extracts the package information from it and then uses this
 * to populate the display models.
 */
void ApplicationData::parseEixData()
{
    std::fstream input(_eixOutput->fileName().toStdString(),
                       std::ios::in | std::ios::binary);
    if (!eix.ParseFromIstream(&input)) {

        qWarning() << "Failed to parse EIX output";
        eix.clear_category();
    }

    // Merge the data for installed packages and eix info together.
    combinedPackageList.load(eix, search());

    setupCategoryTreeModelData();
}

/*!
 * Add the contents of the given category item into the package model.
 * Note that an example of a container category could be "dev", which
 * has sub-categories called "dev-lib", "dev-util", etc. The sub-categories
 * are not classed as containers, but they do have packages. All of the
 * packages under the given category (and any subcategories) are added to
 * the model.
 */
void ApplicationData::addCategory(CategoryTreeItem *catItem)
{
    if (catItem->isContainer()) {
        // Recurse into child nodes
        for (int child = 0; child < catItem->childCount(); ++child) {
            addCategory(catItem->child(child));
        }
    } else {
        const auto &cat = eix.category(catItem->categoryNumber());
        for (int pkgNumber = 0; pkgNumber < cat.package_size(); ++pkgNumber) {
            VersionMap zombieList = combinedPackageList.zombieVersions(
                cat.category(),
                cat.package(pkgNumber).name());
            packageReportModel.addPackage(cat.category(),
                                          cat.package(pkgNumber),
                                          zombieList);
        }
    }
}

/*!
 * Loads all the data that has been parsed from the eix protobuf output
 * into the data model for the category tree.
 */
void ApplicationData::setupCategoryTreeModelData()
{
    // Decode the eix data
    categoryTreeModel.startUpdate();
    categoryTreeModel.clear();

    for (int catNumber = 0; catNumber < eix.category_size(); ++catNumber) {
        const auto &catRef = eix.category(catNumber);
        QString categoryName = catRef.category().c_str();
        categoryTreeModel.addCategory(catNumber,
                                      categoryName,
                                      catRef.package_size());
    }

    categoryTreeModel.endUpdate();

    // emit signal (for MainWindow updates)
    emit categoryModelUpdated();
}

/*!
 * Loads the package model with packages from the given category item tree.
 * This can be a top level category, or a second level category.
 */
void ApplicationData::setupPackageModelData(CategoryTreeItem *catItem)
{
    packageReportModel.startUpdate();
    packageReportModel.clear();

    addCategory(catItem);
    packageReportModel.endUpdate();
}

QString ApplicationData::findRepositoryPath(const QString &name) const
{
    return _repositoryIndex.find(name);
}

/*!
 * Runs "eix --proto" in a separate process
 *
 * This uses eix to generate the 'proto' data into a temporary file. It does
 * not wait for the command to complete; it signals onEixFinished() if the
 * process runs and completes, or onEixError() if it failed to start.
 *
 * If eix succeeds, the eix data is loaded, the portage installed pkg database
 * is read and merged in, and then the display is updated.
 */
void ApplicationData::loadPortageData()
{
    // TODO: check the executable exists, if not then popup and terminate

    emit eixRunning(true);

    _repositoryIndex.load();

    // Create the temporary file for the protobuf data. All we want is the
    // name because its going to be written by the eix process, but to get
    // that, it's necessary to open the temp file. It will be closed when
    // eixOutput is destroyed.
    _eixOutput = new QTemporaryFile();
    _eixOutput->open();

    _eixProcess = new QProcess;
    QStringList eix_params = {"--proto"};

    if (filters()) {
        switch (selectionFilter()) {

        default:
            break;

        case SelectionFilter::All:
            break;

        case SelectionFilter::Installed:
            eix_params << "-I";
            break;

        case SelectionFilter::World:
            eix_params << "--world";
            break;
        }

        if (!search().isEmpty()) {
            // The searchbox validation only allows dashes, letters, digits. It
            // does not allow dashes to be doubled. If the search text starts
            // with "-", need to quote with "--" prefix
            QString leader(search().startsWith("-") ? "--" : "");
            eix_params << leader + search();
        }
    }

    _eixProcess->setStandardOutputFile(_eixOutput->fileName());

    // These signals get triggered by either the process completing, or by not
    // starting. Docs are not really clear on this, but experimentally it seems
    // if the executable file is not found the error happens but NOT the finish.
    // So going to assume they are exclusive. There are some other signals in
    // this class that I've decided to ignore.
    connect(_eixProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            &ApplicationData::onEixFinished);
    connect(_eixProcess,
            &QProcess::errorOccurred,
            this,
            &ApplicationData::onEixError);

    _eixProcess->start(ApplicationData::eixApp, eix_params);
}

/*!
 * Called to cleanup after the eix process has completed.
 * For successful run, this happens after the data has been parsed.
 *
 * Emits a signal to let interested parties know the process is done.
 */
void ApplicationData::cleanupEixProcess()
{
    delete _eixProcess;
    _eixProcess = nullptr;

    delete _eixOutput;
    _eixOutput = nullptr;

    emit eixRunning(false);
}

/*!
 * This event follows a successful launch and the completion of the eix process.
 * The exit code for the process indicates whether the process completed
 * successfully or not.
 */
void ApplicationData::onEixFinished(int exitCode, QProcess::ExitStatus)
{
    if (exitCode == 0) {
        lastLoadTime = QDateTime::currentDateTime();
        parseEixData();
    } else {
        // Eix is reporting an error of some sort. It returns 1 if there is no
        // match, but don't know how specific that error code is. Going to treat
        // all errors as 'no data'.
        // TODO: what error codes are returned by eix and is there any point
        // distinguishing them from 'no data',

        qCritical() << "Calling eix returned error code:" << exitCode;

        eix.clear_category();
        setupCategoryTreeModelData();
    }

    cleanupEixProcess();
}

/*!
 * This event follows a failed launch of the eix process, i.e. it didn't
 * actually start!
 */
void ApplicationData::onEixError(QProcess::ProcessError error)
{
    // Most likely reason would be the exe path not being valid, maybe
    // eix is not installed.
    qCritical() << "Failed to run eix, error code:" << error;

    eix.clear_category();
    setupCategoryTreeModelData();

    cleanupEixProcess();
}
