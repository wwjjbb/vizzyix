// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#ifndef REPOSITORYINDEX_H
#define REPOSITORYINDEX_H

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

#endif // REPOSITORYINDEX_H
