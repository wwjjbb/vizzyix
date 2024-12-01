// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <QDir>
#include <QString>

class CombinedPackageInfo
{
  public:
    CombinedPackageInfo();
    CombinedPackageInfo(const QString &versionName, const QDir &versionDir);

    CombinedPackageInfo(const CombinedPackageInfo &item);
    friend void swap(CombinedPackageInfo &first, CombinedPackageInfo &second);
    CombinedPackageInfo &operator=(CombinedPackageInfo other);

    const QString &versionName() const;
    const QDir &versionDir() const;
    bool inEixDb() const;
    bool inPkgDb() const;

    void setEixDb(bool state);
    void setPkgDb(bool state);
    void setVersionDir(QDir dir);

  private:
    QString oVersionName;
    QDir oVersionDir;
    bool oInEixDb;
    bool oInPkgDb;
};
