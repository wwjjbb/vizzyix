# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

TEMPLATE = subdirs

SUBDIRS += \
    eixpb \
    vizzyix \
    tests

vizzyix.depends = eixpb

tests.depends = eixpb

DISTFILES += \
    meson.build \
    .qmake.conf \
    ChangeLog \
    README \
    AUTHORS \
    notes.txt
