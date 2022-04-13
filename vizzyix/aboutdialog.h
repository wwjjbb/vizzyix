// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui
{
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

  private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
