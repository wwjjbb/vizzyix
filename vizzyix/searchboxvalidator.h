// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QValidator>

class SearchBoxValidator : public QValidator
{
    Q_OBJECT
  public:
    SearchBoxValidator(QObject *parent = nullptr);

    // QValidator interface
  public:
    State validate(QString &input, int &pos) const override;
};
