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
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
