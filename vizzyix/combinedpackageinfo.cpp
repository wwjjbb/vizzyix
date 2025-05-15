// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "combinedpackageinfo.h"

#include <QString>

CombinedPackageInfo::CombinedPackageInfo()
{
}

CombinedPackageInfo::CombinedPackageInfo(const QString &versionName,
                                         const QDir &versionDir)
    : _versionName(versionName), _versionDir(versionDir), _inEixDb(false),
      _inPkgDb(false)
{
}

CombinedPackageInfo::CombinedPackageInfo(const CombinedPackageInfo &item)
    : _versionName(item._versionName), _versionDir(item._versionDir),
      _inEixDb(item._inEixDb), _inPkgDb(item._inPkgDb)
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

    swap(first._versionName, second._versionName);
    swap(first._versionDir, second._versionDir);
    swap(first._inEixDb, second._inEixDb);
    swap(first._inPkgDb, second._inPkgDb);
}

const QString &CombinedPackageInfo::versionName() const
{
    return _versionName;
}

const QDir &CombinedPackageInfo::versionDir() const
{
    return _versionDir;
}

bool CombinedPackageInfo::inEixDb() const
{
    return _inEixDb;
}

bool CombinedPackageInfo::inPkgDb() const
{
    return _inPkgDb;
}

void CombinedPackageInfo::setEixDb(bool state)
{
    _inEixDb = state;
}

void CombinedPackageInfo::setPkgDb(bool state)
{
    _inPkgDb = state;
}

void CombinedPackageInfo::setVersionDir(QDir dir)
{
    _versionDir = dir;
}
