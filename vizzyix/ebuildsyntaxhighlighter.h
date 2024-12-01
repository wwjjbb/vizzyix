// SPDX-FileCopyrightText: 2024 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only
#pragma once

#include <QRegularExpression>
#include <QSyntaxHighlighter>

class EbuildSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
  public:
    EbuildSyntaxHighlighter(QTextDocument *parent = nullptr);
    void highlightBlock(const QString &text);

  private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QList<HighlightingRule> highlightingRules;
    QTextCharFormat keywordFormat;
    QTextCharFormat varFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat functionFormat;
    QTextCharFormat punctuationFormat;
};
