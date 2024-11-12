// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "searchboxvalidator.h"
#include <QRegularExpression>

SearchBoxValidator::SearchBoxValidator(QObject *parent) : QValidator(parent)
{
}

QValidator::State SearchBoxValidator::validate(QString &input, int &) const
{
    // Search strings for now:
    // * May be blank
    // * May contain letters, digits, or '-'
    // * May start with "^"
    // * May finish with "$"
    // * May not contain '--'

    // TODO: add boolean ops translating to "--and" and "--or"
    // (which is why this is not regex)

    if (input.isEmpty())
        return QValidator::Acceptable;

    if (!input.contains("--") &&
        QRegularExpression("^\\^*[\\w\\-]+\\$*$").match(input).hasMatch())
        return QValidator::Acceptable;

    return QValidator::Invalid;
}
