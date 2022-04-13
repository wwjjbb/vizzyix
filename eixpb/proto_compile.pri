# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

# QMAKE rules for generating C++ files from proto files
#
# Usage:
#   ...
#   PROTOS = a.proto b.proto
#   include(protobuf.pri)
#
# Adapted from: https://gist.github.com/buzzySmile/d46f537a810a7d97efc7

# -------------------------------------------------------------------------------------
# Note: it is possible to have multi-line commands but they have to be escaped properly:
#
#   blah.commands = echo hello $$escape_expand(\n) echo world
# -------------------------------------------------------------------------------------


INCLUDEPATH += $$OUT_PWD
DEPENDPATH  += $$OUT_PWD

protobuf_decl.name = protobuf headers
protobuf_decl.input = PROTOS
protobuf_decl.output =${QMAKE_FILE_BASE}.pb.h
protobuf_decl.commands = protoc --cpp_out=. --proto_path=${QMAKE_FILE_IN_PATH} ${QMAKE_FILE_NAME}
protobuf_decl.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += protobuf_decl

protobuf_impl.name = protobuf sources
protobuf_impl.input = PROTOS
protobuf_impl.output = ${QMAKE_FILE_BASE}.pb.cc
protobuf_impl.depends = ${QMAKE_FILE_BASE}.pb.h
protobuf_impl.commands = $$escape_expand(\n)
protobuf_impl.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += protobuf_impl
