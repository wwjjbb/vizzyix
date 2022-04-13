// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#ifndef EIXPROTOHELPER_H
#define EIXPROTOHELPER_H

#include "eix.pb.h"

#include <QString>

class EixProtoHelper
{
  public:
    EixProtoHelper();

    static QString toQString(const eix_proto::KeyFlags &keyflags);
    static QString toQString(const eix_proto::MaskFlags &maskflags);
    static eix_proto::MaskFlags_MaskFlag
    classifyInstallType(const eix_proto::MaskFlags &maskFlags);
    static eix_proto::MaskFlags_MaskFlag
    classifyInstallType(const eix_proto::Version &version);
    static bool isStable(const eix_proto::KeyFlags &keyFlags);
    static bool isStable(const eix_proto::Version &version);
    static QString useFlagSummary(const eix_proto::Version &version);
};

#endif // EIXPROTOHELPER_H
