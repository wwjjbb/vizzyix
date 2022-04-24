# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

MESON_PROJECT_FILE = $$top_srcdir/meson.build

ver_gen.name = version header
ver_gen.input = MESON_PROJECT_FILE
ver_gen.output = version.h
ver_gen.commands = $$top_srcdir/scripts/mkconfig ${QMAKE_FILE_NAME} ${QMAKE_FILE_OUT}
ver_gen.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += ver_gen


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -L../eixpb -leixpb

INCLUDEPATH += $$top_builddir/eixpb
INCLUDEPATH += $$top_srcdir/includes/HtmlBuilder

SOURCES += \
    aboutdialog.cpp \
    applicationdata.cpp \
    categorytreeitem.cpp \
    categorytreemodel.cpp \
    combinedpackageinfo.cpp \
    combinedpackagelist.cpp \
    eixprotohelper.cpp \
    main.cpp \
    mainwindow.cpp \
    packagereportitem.cpp \
    packagereportmodel.cpp \
    repositoryindex.cpp \
    searchboxvalidator.cpp

HEADERS += \
    aboutdialog.h \
    applicationdata.h \
    categorytreeitem.h \
    categorytreemodel.h \
    combinedpackageinfo.h \
    combinedpackagelist.h \
    customhtmlelements.h \
    eixprotohelper.h \
    localexceptions.h \
    mainwindow.h \
    packagereportitem.h \
    packagereportmodel.h \
    repositoryindex.h \
    searchboxvalidator.h

FORMS += \
    aboutdialog.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += protobuf

RESOURCES += \
    resources.qrc

DISTFILES += \
    meson.build \
    version.h.in
