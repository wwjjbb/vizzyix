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

class EbuildSyntaxHighlighter;

class DetailsDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit DetailsDialog(QWidget *parent = nullptr);
    ~DetailsDialog();

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

  private:
    enum Tab {
        Summary,
        Ebuild,
        InstalledFiles,
        UseFlags,
    };

  private:
    Ui::DetailsDialog *ui;
    QStandardItemModel _installedFiles;
    QFile _repoEbuildFile;
    QDir _pkgDir;
    QString _repository;
    QString _category;
    QString _package;
    QString _version;
    EbuildSyntaxHighlighter *_highlighter;
};
