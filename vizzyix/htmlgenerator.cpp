// SPDX-FileCopyrightText: 2024 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "htmlgenerator.h"

HtmlGenerator::HtmlGenerator()
{
}

void HtmlGenerator::clear()
{
    lines.clear();
}

bool HtmlGenerator::isEmpty() const
{
    return lines.isEmpty();
}

QString HtmlGenerator::toString() const
{
    return lines.join('\n');
}

void HtmlGenerator::text(const QString &txt)
{
    lines.append(txt);
}

void HtmlGenerator::header(int level, const QString &txt)
{
    lines.append(QString("<h%1>%2</h%1>").arg(level).arg(txt));
}

void HtmlGenerator::para(const QString &txt)
{
    lines.append(QString("<p>%1</p>").arg(txt));
}

void HtmlGenerator::link(const QString &link, const QString &txt)
{
    lines.append(QString("<a href=\"%1\">%2</a>").arg(link).arg(txt));
}

void HtmlGenerator::col(const QString &txt)
{
    lines.append(QString("<td>%1</td>").arg(txt));
}

void HtmlGenerator::image(const QString &link,
                          const QString &alt,
                          int width,
                          int height)
{
    lines.append(QString("<img src=\"%1\" alt=\"%2\" width=%3 height=%4>")
                     .arg(link)
                     .arg(alt)
                     .arg(width)
                     .arg(height));
}

void HtmlGenerator::hr()
{
    start("hr");
}

void HtmlGenerator::startHtml()
{
    lines.append("<!DOCTYPE html>");
    start("html");
    start("body");
}

void HtmlGenerator::endHtml()
{
    end("body");
    end("html");
}

void HtmlGenerator::startTable()
{
    start("table");
}

void HtmlGenerator::endTable()
{
    end("table");
}

void HtmlGenerator::startRow()
{
    start("tr");
}

void HtmlGenerator::endRow()
{
    end("tr");
}

void HtmlGenerator::startCol()
{
    start("td");
}

void HtmlGenerator::endCol()
{
    end("td");
}

void HtmlGenerator::startColHead()
{
    start("th");
}

void HtmlGenerator::endColHead()
{
    end("th");
}

void HtmlGenerator::start(const QString &tag)
{
    lines.append(QString("<%1>").arg(tag));
}

void HtmlGenerator::end(const QString &tag)
{
    lines.append(QString("</%1>").arg(tag));
}
