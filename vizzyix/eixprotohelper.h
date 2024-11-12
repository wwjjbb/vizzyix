// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "eix.pb.h"

#include <QString>

// Version Data
//   id()                    e.g. "2.8.2"
//   keywords()              e.g. "amd64 ~arm ~arm64 ~riscv x86"
//   keywords_effective()    dunno, none found
//   local_mask_flags()      World / System / Set
//   system_mask_flags()
//   local_key_flags()       "STABLE, ARCH-STABLE, ALIEN-STABLE, ALIEN-UNSTABLE"
//   system_key_flags()
//   depend()               e.g. "dev-qt/qtbase:6[dbus?,network?] qml? (
//   dev-qt/qtbase:6[gui] dev-qt/qtdeclarative:6= ) websockets? (
//   dev-qt/qtwebsockets:6 ) examples? (
//   dev-qt/qtbase:6[concurrent,network,widgets] ) test? (
//   dev-qt/qtbase:6[concurrent] )"

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
