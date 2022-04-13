# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

QT += testlib

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17

TEMPLATE = app

DEFINES += TESTDATA=\\\"$$top_srcdir/pbtesting/eix.pb\\\"

SOURCES +=  tst_testpackagereportmodel.cpp \
    ../../vizzyix/eixprotohelper.cpp \
    ../../vizzyix/packagereportitem.cpp \
    ../../vizzyix/combinedpackageinfo.cpp \
    ../../vizzyix/combinedpackagelist.cpp \
    ../../vizzyix/packagereportmodel.cpp

LIBS += -L../../eixpb -leixpb

INCLUDEPATH += $$top_builddir/eixpb ../../vizzyix

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += protobuf

HEADERS += \
    ../../vizzyix/eixprotohelper.h \
    ../../vizzyix/packagereportitem.h \
    ../../vizzyix/packagereportmodel.h \
    ../../vizzyix/combinedpackageinfo.h \
    ../../vizzyix/combinedpackagelist.h

DISTFILES += \
    meson.build

