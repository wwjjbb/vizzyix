# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17

TEMPLATE = app

SOURCES +=  tst_testcategorytreeitem.cpp \
    ../../vizzyix/categorytreeitem.cpp

LIBS += -L../../eixpb -leixpb

INCLUDEPATH += $$top_builddir/eixpb ../../vizzyix

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += protobuf

HEADERS += \
    ../../vizzyix/categorytreeitem.h

DISTFILES += \
    meson.build
