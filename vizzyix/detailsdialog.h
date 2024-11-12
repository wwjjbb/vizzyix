// SPDX-FileCopyrightText: 2024 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "applicationdata.h"
#include <QDialog>
#include <QStandardItemModel>

namespace Ui
{
class DetailsDialog;
}

class DetailsDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit DetailsDialog(QWidget *parent = nullptr);
    ~DetailsDialog();

  private:
    enum Tab {
        Summary,
        Ebuild,
        InstalledFiles,
        UseFlags,
    };

  private:
    Ui::DetailsDialog *ui;
    QStandardItemModel installedFiles;
    QFile repoEbuildFile;
    QDir pkgDir;
    QString repository;
    QString category;
    QString package;
    QString version;

    void updateDetails();
    void updateEbuildTab();
    void updateInstalledFilesTab();
    void updateUseFlagsTab();

  public slots:
    void tabChanged(int newTab);
    void showEbuild(const QString &repository,
                    const QString &category,
                    const QString &package,
                    const QString &version);
};
