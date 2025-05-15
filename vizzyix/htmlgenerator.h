// SPDX-FileCopyrightText: 2024 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QStringList>

class HtmlGenerator
{
  public:
    HtmlGenerator();
    void clear();
    bool isEmpty() const;
    QString toString() const;
    void text(const QString &txt);
    void header(int level, const QString &txt);
    void para(const QString &txt);
    void link(const QString &link, const QString &txt);
    void col(const QString &txt);
    void image(const QString &link, const QString &alt, int width, int height);
    void hr();
    void startHtml();
    void endHtml();
    void startTable();
    void endTable();
    void startRow();
    void endRow();
    void startCol();
    void endCol();
    void startColHead();
    void endColHead();

  private:
    void start(const QString &tag);
    void end(const QString &tag);
    QStringList _lines;
};
