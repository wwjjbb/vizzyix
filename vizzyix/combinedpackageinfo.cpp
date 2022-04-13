// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "combinedpackageinfo.h"

#include <QString>

CombinedPackageInfo::CombinedPackageInfo()
{
}

CombinedPackageInfo::CombinedPackageInfo(const QString &versionName,
                                         const QDir &versionDir)
    : versionName_(versionName), versionDir_(versionDir), inEixDb_(false),
      inPkgDb_(false)
{
}

CombinedPackageInfo::CombinedPackageInfo(const CombinedPackageInfo &item)
    : versionName_(item.versionName_), versionDir_(item.versionDir_),
      inEixDb_(item.inEixDb_), inPkgDb_(item.inPkgDb_)
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

    swap(first.versionName_, second.versionName_);
    swap(first.versionDir_, second.versionDir_);
    swap(first.inEixDb_, second.inEixDb_);
    swap(first.inPkgDb_, second.inPkgDb_);
}

const QString &CombinedPackageInfo::versionName() const
{
    return versionName_;
}

const QDir &CombinedPackageInfo::versionDir() const
{
    return versionDir_;
}

bool CombinedPackageInfo::inEixDb() const
{
    return inEixDb_;
}

bool CombinedPackageInfo::inPkgDb() const
{
    return inPkgDb_;
}

void CombinedPackageInfo::setEixDb(bool state)
{
    inEixDb_ = state;
}

void CombinedPackageInfo::setPkgDb(bool state)
{
    inPkgDb_ = state;
}

void CombinedPackageInfo::setVersionDir(QDir dir)
{
    versionDir_ = dir;
}
