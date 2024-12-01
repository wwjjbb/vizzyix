// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "combinedpackageinfo.h"

#include <QString>

CombinedPackageInfo::CombinedPackageInfo()
{
}

CombinedPackageInfo::CombinedPackageInfo(const QString &versionName,
                                         const QDir &versionDir)
    : oVersionName(versionName), oVersionDir(versionDir), oInEixDb(false),
      oInPkgDb(false)
{
}

CombinedPackageInfo::CombinedPackageInfo(const CombinedPackageInfo &item)
    : oVersionName(item.oVersionName), oVersionDir(item.oVersionDir),
      oInEixDb(item.oInEixDb), oInPkgDb(item.oInPkgDb)
{
}

CombinedPackageInfo &CombinedPackageInfo::operator=(CombinedPackageInfo other)
{
    swap(*this, other);
    return *this;
}

void swap(CombinedPackageInfo &first, CombinedPackageInfo &second)
{
    using std::swap;

    swap(first.oVersionName, second.oVersionName);
    swap(first.oVersionDir, second.oVersionDir);
    swap(first.oInEixDb, second.oInEixDb);
    swap(first.oInPkgDb, second.oInPkgDb);
}

const QString &CombinedPackageInfo::versionName() const
{
    return oVersionName;
}

const QDir &CombinedPackageInfo::versionDir() const
{
    return oVersionDir;
}

bool CombinedPackageInfo::inEixDb() const
{
    return oInEixDb;
}

bool CombinedPackageInfo::inPkgDb() const
{
    return oInPkgDb;
}

void CombinedPackageInfo::setEixDb(bool state)
{
    oInEixDb = state;
}

void CombinedPackageInfo::setPkgDb(bool state)
{
    oInPkgDb = state;
}

void CombinedPackageInfo::setVersionDir(QDir dir)
{
    oVersionDir = dir;
}
