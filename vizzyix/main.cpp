// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // Set defaults for QSettings
    QCoreApplication::setOrganizationName("ThingsEtc");
    QCoreApplication::setApplicationName("vizzyix");

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/app/images/eyeball.png"));
    MainWindow w;
    w.show();
    return a.exec();
}
