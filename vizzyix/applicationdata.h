// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QDateTime>
#include <QObject>
#include <QProcess>
#include <QTemporaryFile>

#include "categorytreemodel.h"
#include "combinedpackagelist.h"
#include "eix.pb.h"
#include "packagereportmodel.h"
#include "repositoryindex.h"

class ApplicationData : public QObject
{
    Q_OBJECT
  private:
    /// Private constructor to ensure only one instance
    ApplicationData();

  public:
    static ApplicationData *data();

    /// Defines the main filter types
    enum SelectionFilter { All, Installed, World };

    void setFilters(bool on);
    bool filters();
    void setSelectionFilter(SelectionFilter filter);
    SelectionFilter selectionFilter();
    void setSearch(const QString &search = "");
    const QString search();

    void parseEixData();
    void setupCategoryTreeModelData();
    void setupPackageModelData(CategoryTreeItem *catItem);

  public:
    static constexpr auto eixApp = "/usr/bin/eix";
    static constexpr auto emergeLogFile = "/var/log/emerge.log";
    static constexpr auto portageEixFile = "/var/cache/eix/portage.eix";
    static constexpr auto reposConfFile = "/etc/portage/repos.conf";
    static constexpr auto packageDatabaseRoot = "/var/db/pkg";

  public:
    /// Manages the list of known repositories and their locations
    RepositoryIndex repositoryIndex;

    /// When the eix database was last loaded into memory
    QDateTime lastLoadTime;

    /// The protobuf copy of the eix database. This may be filtered.
    eix_proto::Collection eix;

    /// A list of all known packages, generated from the eix data
    /// and overlaid with the package database (installed packages)
    CombinedPackageList packageList{packageDatabaseRoot};

    /// The data model for the category tree (shown at left top)
    CategoryTreeModel categoryTreeModel;

    /// The data model for the package report list (shown at top right)
    PackageReportModel packageModel;

  signals:
    void eixRunning(bool running);
    void categoryModelUpdated();

  public slots:
    void loadPortageData();

  private:
    void cleanupEixProcess();
    void addCategory(CategoryTreeItem *catItem);

  private:
    /// The current search filter string
    QString search_{""};

    /// The top level filter
    SelectionFilter selectionFilter_{All};

    /// The handle for the eix process
    QProcess *eixProcess = nullptr;

    /// Somewhere to catch the output from the eix process.
    /// The content of the file is protobuf data.
    QTemporaryFile *eixOutput = nullptr;

  private:
    /// The single instance of this class.
    /// The unique_ptr ensures the object is properly disposed.
    static std::unique_ptr<ApplicationData> appData_;

  private slots:
    void onEixFinished(int exitCode, QProcess::ExitStatus);
    void onEixError(QProcess::ProcessError error);
};
