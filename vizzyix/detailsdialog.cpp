// SPDX-FileCopyrightText: 2024 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "detailsdialog.h"
#include "ui_detailsdialog.h"

#include <QDebug>

// TODO - Implement Summary Tab
// TODO - Implement Installed Files Tab
// TODO - Implement Ebuild Tab
// TODO - Implement USE Flags Tab
// TODO - Implement Build Times Tab?

DetailsDialog::DetailsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::DetailsDialog)
{
    ui->setupUi(this);
    connect(ui->tabWidget,
            &QTabWidget::currentChanged,
            this,
            &DetailsDialog::tabChanged);

    // Ensure first tab is set initially
    ui->tabWidget->setCurrentIndex(0);
}

DetailsDialog::~DetailsDialog()
{
    delete ui;
}

void DetailsDialog::updateDetails()
{
    ui->textSummary->clear();
    ui->textSummary->append(repoEbuildFile.fileName());

    bool installed = pkgDir.exists();
    if (installed) {
        ui->textSummary->append(pkgDir.absolutePath());
    }

    int current = ui->tabWidget->currentIndex();

    if (current == Tab::Ebuild)
        updateEbuildTab();
    else if (current == Tab::InstalledFiles)
        updateInstalledFilesTab();
    else if (current == Tab::UseFlags)
        updateUseFlagsTab();
}

void DetailsDialog::updateEbuildTab()
{
    ui->textEbuild->clear();
    QTextCursor top = ui->textEbuild->textCursor();
    repoEbuildFile.open(QIODevice::Text | QIODevice::ReadOnly);
    while (!repoEbuildFile.atEnd()) {
        QString line = repoEbuildFile.readLine();
        if (line.endsWith('\n')) {
            line.chop(1);
        }
        ui->textEbuild->append(line);
    }
    ui->textEbuild->moveCursor(QTextCursor::Start);
    ui->textEbuild->ensureCursorVisible();
    repoEbuildFile.close();
}

void DetailsDialog::updateInstalledFilesTab()
{
    QFile contents{pkgDir.filePath("CONTENTS")};
    installedFiles.clear();
    if (contents.exists()) {
        contents.open(QIODevice::Text | QIODevice::ReadOnly);
        while (!contents.atEnd()) {
            QString line = contents.readLine();
            if (line.endsWith('\n')) {
                line.chop(1);
            }
            QStringList fields = line.split(" ");
            // qDebug() << fields[1];
            if (fields[0] == "obj" || fields[0] == "dir") {
                installedFiles.appendRow(new QStandardItem(fields[1]));
            } else if (fields[0] == "sym") {
                installedFiles.appendRow(
                    new QStandardItem(QString("%1 %2 %3")
                                          .arg(fields[1])
                                          .arg(fields[2])
                                          .arg(fields[3])));
            }
        }
        contents.close();
        installedFiles.sort(0);
        ui->tableInstalledFiles->setModel(&installedFiles);
    }
}

void DetailsDialog::updateUseFlagsTab()
{
    // TODO - Use Flags Tab (call "equery uses <package>"?)
}

void DetailsDialog::tabChanged(int newTab)
{
    updateDetails();
}

void DetailsDialog::showEbuild(const QString &repository,
                               const QString &category,
                               const QString &package,
                               const QString &version)
{
    bool changed =
        (this->repository != repository || this->category != category ||
         this->package != package || this->version != version);

    this->repository = repository;
    this->category = category;
    this->package = package;
    this->version = version;

    if (changed) {
        QString repoDir =
            ApplicationData::data()->repositoryIndex.find(repository);
        QString ebuildfile = QString("%1/%2/%3/%3-%4.ebuild")
                                 .arg(repoDir)
                                 .arg(category)
                                 .arg(package)
                                 .arg(version);
        repoEbuildFile.setFileName(ebuildfile);
        QString pkgDirName = QString("%1/%2/%3-%4")
                                 .arg(ApplicationData::packageDatabaseRoot)
                                 .arg(category, package, version);
        pkgDir.setPath(pkgDirName);
        updateDetails();
        show();
    }
}