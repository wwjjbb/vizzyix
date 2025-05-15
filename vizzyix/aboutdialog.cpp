// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "version.h"

#include <QGraphicsScene>
#include <QGraphicsSvgItem>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    QGraphicsScene *scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    scene->setBackgroundBrush(QBrush(Qt::lightGray));
    auto item = new QGraphicsSvgItem(":/app/images/eyeball.svg");
    scene->addItem(item);
    ui->graphicsView->fitInView(scene->itemsBoundingRect(),
                                Qt::KeepAspectRatioByExpanding);
    ui->graphicsView->scale(2.5, 2.5);

    ui->versionLabel->setText("Version: " APPVERSION);

    ui->tab->setCurrentIndex(0);

    // Connect the URL-clicked signals

    connect(ui->licenseInfo,
            QOverload<const QUrl &>::of(&QTextBrowser::anchorClicked),
            this,
            &AboutDialog::onLinkClicked);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::onLinkClicked(const QUrl &)
{
    // Not used, but would allow local links to be handled.
    // Two examples - diverting a link with a specific filetype
    // to show a local file (in the control), and how to catch
    // a 'back' button.

    /*
     QString filetype = QFileInfo(url.fileName()).suffix();
     if (filetype == "ebuild") {
         QString filename = QFileInfo(url.fileName()).fileName();
         showEbuildSource(url, filename);
     }
     if (url.path() == "BACK" && html != nullptr) {
         ui->packageDescription->setHtml(
             QString::fromStdString(html->toString()));
     }
    */
}
