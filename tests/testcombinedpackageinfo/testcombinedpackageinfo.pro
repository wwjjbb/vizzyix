# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17

TEMPLATE = app

SOURCES +=  tst_testcombinedpackageinfo.cpp \
    ../../vizzyix/combinedpackageinfo.cpp

INCLUDEPATH += ../../vizzyix

DISTFILES += \
    meson.build

HEADERS += \
    ../../vizzyix/combinedpackageinfo.h

