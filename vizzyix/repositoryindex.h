// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QMap>
#include <QString>

/*! class RepositoryIndex
 *
 * Manages a list of repositories from /etc/portage/repos.conf (and
 * the old layman config). Able to find the containing directory for
 * any repository by name.
 */
class RepositoryIndex
{
  public:
    RepositoryIndex();

    bool load();
    QString mainRepo() const;
    QStringList repoList() const;
    QString find(const QString &repo) const;

  private:
    /// The key is repository name, & value is full path to repository directory
    QMap<QString, QString> repoMap_;

    /// The name of the default repository
    QString mainRepo_;
};
