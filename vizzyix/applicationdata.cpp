// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "applicationdata.h"

#include <QDebug>
#include <QProcess>
#include <QTimer>
#include <fstream>
#include <iostream>

ApplicationData::ApplicationData()
{
}

// Whether any filters are currently set
bool ApplicationData::filters()
{
    return selectionFilter() || !search().isEmpty();
}

/*
 * ApplicationData::setSelectionFilter
 *
 * Sets the selection filter to the given value.
 */
void ApplicationData::setSelectionFilter(SelectionFilter filter)
{
    selectionFilter_ = filter;
}

/*
 * ApplicationData::selectionFilter
 *
 * Gets the selection filter value.
 */
ApplicationData::SelectionFilter ApplicationData::selectionFilter()
{
    return selectionFilter_;
}

/*
 * ApplicationData::setSearch
 *
 * Sets the search filter to the given value.
 */
void ApplicationData::setSearch(const QString &search)
{
    search_ = search;
}

const QString ApplicationData::search()
{
    return search_;
}

/*
 * ApplicationData::runEix
 *
 * Runs "eix --proto" in a separate process
 *
 * This uses eix to generate the 'proto' data into a temporary file. It does
 * not wait for the command to complete; it signals onEixFinished() if the
 * process runs and completes, or onEixError() if it failed to start.
 *
 * If eix succeeds, the eix data is loaded, the portage installed pkg database
 * is read and merged in, and then the display is updated.
 */
void ApplicationData::runEix()
{
    emit eixRunning(true);

    // Create the temporary file for the protobuf data. All we want is the
    // name because its going to be written by the eix process, but to get
    // that, it's necessary to open the temp file. It will be closed when
    // eixOutput is destroyed.
    eixOutput = new QTemporaryFile();
    eixOutput->open();

    eixProcess = new QProcess;
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

    eixProcess->setStandardOutputFile(eixOutput->fileName());

    // These signals get triggered by either the process completing, or by not
    // starting. Docs are not really clear on this, but experimentally it seems
    // if the executable file is not found the error happens but NOT the finish.
    // So going to assume they are exclusive. There are some other signals in
    // this class that I've decided to ignore.
    connect(eixProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            &ApplicationData::onEixFinished);
    connect(eixProcess, &QProcess::errorOccurred, this,
            &ApplicationData::onEixError);

    // TODO: configure this path somewhere
    // TODO: check the executable exists
    eixProcess->start("/usr/bin/eix", eix_params);
}

void ApplicationData::parseEixData()
{
    std::fstream input(eixOutput->fileName().toStdString(),
                       std::ios::in | std::ios::binary);
    if (!eix.ParseFromIstream(&input)) {
        // TODO: report this better
        qDebug() << "Failed to parse EIX output";
        eix.clear_category();
    }

    // Merge the data for installed packages and eix info together.

    // This is purely to get the portage package data loaded (i.e. what's
    // installed) and combined with what eix knows about available packages.
    packageList.clear();
    packageList.readEixData(eix);
    packageList.readPortagePackageDatabase(filters());
    packageList.identifyZombies();

    setupCategoryTreeModelData();
}

/*
 * ApplicationData::addCategory
 *
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
            VersionMap zombieList = packageList.zombies(
                cat.category(), cat.package(pkgNumber).name());
            packageModel.addPackage(cat.category(), cat.package(pkgNumber),
                                    zombieList);
        }
    }
}

void ApplicationData::setupCategoryTreeModelData()
{
    // Decode the eix data
    categoryTreeModel.startUpdate();
    categoryTreeModel.clear();

    for (int catNumber = 0; catNumber < eix.category_size(); ++catNumber) {
        const auto &catRef = eix.category(catNumber);
        QString categoryName = catRef.category().c_str();
        categoryTreeModel.addCategory(catNumber, categoryName,
                                      catRef.package_size());
    }

    categoryTreeModel.endUpdate();

    // emit signal (for MainWindow updates)
    emit categoryModelUpdated();
}

/*
 * ApplicationData::setupPackageModelData
 *
 * Loads the package model with packages from the given category
 */
void ApplicationData::setupPackageModelData(CategoryTreeItem *catItem)
{
    packageModel.startUpdate();
    packageModel.clear();
    addCategory(catItem);
    packageModel.endUpdate();
}

void ApplicationData::loadPortageData()
{
    runEix();
}

void ApplicationData::cleanupEixProcess()
{
    delete eixProcess;
    eixProcess = nullptr;

    delete eixOutput;
    eixOutput = nullptr;

    emit eixRunning(false);
}

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
        qDebug() << "Calling eix returned error code" << exitCode;

        eix.clear_category();
        setupCategoryTreeModelData();
    }

    cleanupEixProcess();
}

void ApplicationData::onEixError(QProcess::ProcessError error)
{
    // TODO: report this better
    // Most likely reason would be the exe path not being valid, maybe
    // eix is not installed.
    qDebug() << "Failed to run eix:" << error;

    eix.clear_category();
    setupCategoryTreeModelData();

    cleanupEixProcess();
}
