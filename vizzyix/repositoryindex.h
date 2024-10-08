// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QMap>
#include <QString>

class RepositoryIndex
{
  public:
    RepositoryIndex();

    bool load();
    QString mainRepo() const;
    QStringList repoList() const;
    QString find(const QString &repo) const;

  private:
    void checkMakeConf();
    static const QString reposConf;

    QMap<QString, QString> repoMap_;
    QString mainRepo_;
};
