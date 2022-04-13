# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

QT -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# TODO: add configuration option for this path
PROTOPATH = /usr/share/eix
PROTOS = $$PROTOPATH/eix.proto
include(proto_compile.pri)

DISTFILES += \
    meson.build \
    proto_compile.pri

