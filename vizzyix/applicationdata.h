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
  public:
    ApplicationData();

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
    RepositoryIndex repositoryIndex;
    QDateTime lastLoadTime;

    eix_proto::Collection eix;
    CombinedPackageList packageList;

    CategoryTreeModel categoryTreeModel;
    PackageReportModel packageModel;

  signals:
    void eixRunning(bool running);
    void categoryModelUpdated();

  public slots:
    void loadPortageData();
    void runEix();

  private:
    void cleanupEixProcess();
    void addCategory(CategoryTreeItem *catItem);

  private:
    QString search_{""};
    SelectionFilter selectionFilter_{All};
    QProcess *eixProcess = nullptr;
    QTemporaryFile *eixOutput = nullptr;

  private slots:
    void onEixFinished(int exitCode, QProcess::ExitStatus);
    void onEixError(QProcess::ProcessError error);
};
