// SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
// SPDX-License-Identifier: GPL-2.0-only

#include "eixprotohelper.h"

#include <QString>
#include <QStringList>

EixProtoHelper::EixProtoHelper()
{
}

// I don't have any packages with minus or asterisk, so I don't care what this
// means at this time. Also not got a scooby about the distinction between
// local and system key flags because all the ones I've seen have been the same,
// and the eix docs do not appear to explain. And a brief look at eix code got
// lost quite quick.
// Going to assume that 'local' is representative of the local (i.e. my) config.
QString EixProtoHelper::toQString(const eix_proto::KeyFlags &keyflags)
{
    QStringList flags;
    for (int flag = 0; flag < keyflags.key_flag_size(); ++flag) {
        switch (keyflags.key_flag(flag)) {
        case eix_proto::KeyFlags_KeyFlag_UNKNOWN:
            flags.append("UNKNOWN");
            break;
        case eix_proto::KeyFlags_KeyFlag_STABLE:
            flags.append("STABLE");
            break;
        case eix_proto::KeyFlags_KeyFlag_ARCHSTABLE:
            flags.append("ARCH-STABLE");
            break;
        case eix_proto::KeyFlags_KeyFlag_ARCHUNSTABLE:
            flags.append("ARCH-UNSTABLE");
            break;
        case eix_proto::KeyFlags_KeyFlag_ALIENSTABLE:
            flags.append("ALIEN-STABLE");
            break;
        case eix_proto::KeyFlags_KeyFlag_ALIENUNSTABLE:
            flags.append("ALIEN-UNSTABLE");
            break;
        case eix_proto::KeyFlags_KeyFlag_MINUSKEYWORD:
            flags.append("MINUS-KEYWORD");
            break;
        case eix_proto::KeyFlags_KeyFlag_MINUSUNSTABLE:
            flags.append("MINUS-UNSTABLE");
            break;
        case eix_proto::KeyFlags_KeyFlag_MINUSASTERISK:
            flags.append("MINUS-ASTERISK");
            break;
        default:
            flags.append("(Undefined Key Flag)");
            break;
        }
    }
    return flags.join(", ");
}

QString EixProtoHelper::toQString(const eix_proto::MaskFlags &maskflags)
{
    QStringList flags;
    for (int flag = 0; flag < maskflags.mask_flag_size(); ++flag) {
        switch (maskflags.mask_flag(flag)) {
        case eix_proto::MaskFlags_MaskFlag_UNKNOWN:
            flags.append("UNKNOWN");
            break;
        case eix_proto::MaskFlags_MaskFlag_MASK_PACKAGE:
            flags.append("MASK-PACKAGE");
            break;
        case eix_proto::MaskFlags_MaskFlag_MASK_SYSTEM:
            // set of the package is in the @system set
            flags.append("MASK-SYSTEM");
            break;
        case eix_proto::MaskFlags_MaskFlag_MASK_PROFILE:
            flags.append("MASK-PROFILE");
            break;
        case eix_proto::MaskFlags_MaskFlag_IN_PROFILE:
            flags.append("IN-PROFILE");
            break;
        case eix_proto::MaskFlags_MaskFlag_WORLD:
            // set if the package is in the @world set
            flags.append("WORLD");
            break;
        case eix_proto::MaskFlags_MaskFlag_WORLD_SETS:
            // set if the package is in a user defined set
            flags.append("WORLD-SETS");
            break;
        case eix_proto::MaskFlags_MaskFlag_MARKED:
            flags.append("MARKED");
            break;
        default:
            flags.append("(Undefined Mask Flag)");
            break;
        }
    }
    return flags.join(", ");
}

eix_proto::MaskFlags_MaskFlag
EixProtoHelper::classifyInstallType(const eix_proto::MaskFlags &maskFlags)
{
    for (int flagNumber = 0; flagNumber < maskFlags.mask_flag_size();
         ++flagNumber) {
        auto flag = maskFlags.mask_flag(flagNumber);

        if (flag == eix_proto::MaskFlags_MaskFlag_WORLD ||
            flag == eix_proto::MaskFlags_MaskFlag_WORLD_SETS ||
            flag == eix_proto::MaskFlags_MaskFlag_MASK_SYSTEM) {
            return flag;
        }
    }
    return eix_proto::MaskFlags_MaskFlag_UNKNOWN;
}

eix_proto::MaskFlags_MaskFlag
EixProtoHelper::classifyInstallType(const eix_proto::Version &version)
{
    // TODO: what is the difference between the local and system flags.
    // The values are mostly (but not always) the same. For now check
    // both sets.
    //
    // eix --world includes things with the following flags, so this
    // function should too:
    //
    // WORLD - true for everything I have installed, and not seen for
    // anything else
    //
    // WORLD-SETS - true for everything I have in my custom world set,
    // nothing else.
    //
    // MASK-SYSTEM - true for system things, e.g. make. Eix includes
    // these in --world. There are other world options but I think
    // this one gives best results.

    auto result = eix_proto::MaskFlags_MaskFlag_UNKNOWN;
    if (version.has_local_mask_flags()) {
        result =
            EixProtoHelper::classifyInstallType(version.local_mask_flags());
    }
    if (result == eix_proto::MaskFlags_MaskFlag_UNKNOWN &&
        version.has_system_mask_flags()) {
        result =
            EixProtoHelper::classifyInstallType(version.system_mask_flags());
    }
    return result;
}

bool EixProtoHelper::isStable(const eix_proto::KeyFlags &keyFlags)
{
    for (int flagNumber = 0; flagNumber < keyFlags.key_flag_size();
         ++flagNumber) {
        auto flag = keyFlags.key_flag(flagNumber);

        if (flag == eix_proto::KeyFlags_KeyFlag_ARCHSTABLE) {
            return true;
        }
    }
    return false;
}

bool EixProtoHelper::isStable(const eix_proto::Version &version)
{
    if (version.has_local_key_flags() && isStable(version.local_key_flags())) {
        return true;
    }
    if (version.has_system_key_flags() &&
        isStable(version.system_key_flags())) {
        return true;
    }
    return false;
}

// Makes a list of the use flags for a version, as defined in the ebuild.
// The flags are sorted alphabetically; Flags that default ON are prefixed with
// "+", and those that default OFF are prefixed with "-".
QString EixProtoHelper::useFlagSummary(const eix_proto::Version &version)
{
    QStringList uses;

    int iuse_index = 0;
    int iuse_plus_index = 0;
    int iuse_minus_index = 0;

    int selected;
    do {
        QString candidate;
        selected = 0;

        if (iuse_index < version.iuse_size()) {
            QString test = QString::fromStdString(version.iuse(iuse_index));
            if (selected == 0 || test.compare(candidate) < 0) {
                candidate = test;
                selected = 1;
            }
        }

        if (iuse_plus_index < version.iuse_plus_size()) {
            QString test =
                QString::fromStdString(version.iuse_plus(iuse_plus_index));
            if (selected == 0 || test.compare(candidate) < 0) {
                candidate = test;
                selected = 2;
            }
        }

        if (iuse_minus_index < version.iuse_minus_size()) {
            QString test =
                QString::fromStdString(version.iuse_minus(iuse_minus_index));
            if (selected == 0 || test.compare(candidate) < 0) {
                candidate = test;
                selected = 3;
            }
        }

        switch (selected) {
        case 1:
            iuse_index++;
            uses << candidate;
            break;

        case 2:
            iuse_plus_index++;
            uses << ("+" + candidate);
            break;

        case 3:
            iuse_minus_index++;
            uses << ("-" + candidate);
            break;

        default:
            break;
        }
    } while (selected != 0);

    return (uses.length() > 0) ? uses.join(" ") : "";
}
