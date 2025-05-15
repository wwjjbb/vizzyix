// SPDX-FileCopyrightText: 2024 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "ebuildsyntaxhighlighter.h"

EbuildSyntaxHighlighter::EbuildSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Note: Later rules are painted on top of earlier rules

#define BX "\\b"

    _keywordFormat.setForeground(Qt::darkBlue);
    _keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral(BX "case" BX),     QStringLiteral(BX "coproc" BX),
        QStringLiteral(BX "do" BX),       QStringLiteral(BX "done" BX),
        QStringLiteral(BX "elif" BX),     QStringLiteral(BX "else" BX),
        QStringLiteral(BX "esac" BX),     QStringLiteral(BX "for" BX),
        QStringLiteral(BX "function" BX), QStringLiteral(BX "if" BX),
        QStringLiteral(BX "in" BX),       QStringLiteral(BX "inherit" BX),
        QStringLiteral(BX "fi" BX),       QStringLiteral(BX "local" BX),
        QStringLiteral(BX "select" BX),   QStringLiteral(BX "then" BX),
        QStringLiteral(BX "time" BX),     QStringLiteral(BX "until" BX),
        QStringLiteral(BX "while" BX),
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = _keywordFormat;
        _highlightingRules.append(rule);
    }

    _punctuationFormat.setForeground(Qt::black);
    _punctuationFormat.setFontWeight(QFont::Bold);
    const QString punctuationPatterns[] = {
        QStringLiteral(">="),
        QStringLiteral("<="),
        QStringLiteral(">"),
        QStringLiteral("<"),
        QStringLiteral("="),
        QStringLiteral("{"),
        QStringLiteral("}"),
        QStringLiteral("\\[\\["),
        QStringLiteral("\\]\\]"),
        QStringLiteral("!"),
    };
    for (const QString &pattern : punctuationPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = _punctuationFormat;
        _highlightingRules.append(rule);
    }

    _commentFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegularExpression("#.*");
    rule.format = _commentFormat;
    _highlightingRules.append(rule);

    // Handles embedded \"
    // Anything enclosed in ".....", (anything not dquotes or slash) or
    // (slash+anything)
    _stringFormat.setForeground(Qt::darkGreen);
    rule.pattern =
        QRegularExpression(QStringLiteral("\"(?:[^\"\\\\]|\\\\.)*\""));
    rule.format = _stringFormat;
    _highlightingRules.append(rule);

    _functionFormat.setForeground(Qt::darkRed);
    rule.pattern = QRegularExpression("^\\s*\\w+\\(\\)");
    rule.format = _functionFormat;
    _highlightingRules.append(rule);

    _varFormat.setForeground(Qt::darkMagenta);
    _varFormat.setFontWeight(QFont::Bold);
    const QString varPatterns[] = {
        QStringLiteral("^\\s*\\w+="),
        QStringLiteral("^\\s*\\w+\\+="),
        QStringLiteral("\\${\\w+}"),
    };
    for (const QString &pattern : varPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = _varFormat;
        _highlightingRules.append(rule);
    }

    // TODO Highlight variables
    // $<varname> ${<varname>} ^\s*<varname>[=|+=]

    // TODO Highlight multiline strings
    // Theres an example for multi-line comments
    // Need a way to insert into the rules list, so can get correct priority
    // Cases:
    //   BS0: ....."....\"....".... -> BS0 (string on a line, handled by a
    //   normal rule) BS0: ....."....\"........$ -> BS1 (highlight from " to
    //   eoln) BS1: ..........\"........$ -> BS1 (highlight entire line) BS1:
    //   ..........\"..."....$ -> BS0 (highlight start line to ")
}

void EbuildSyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : std::as_const(_highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator =
            rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(),
                      match.capturedLength(),
                      rule.format);
        }
    }
}
