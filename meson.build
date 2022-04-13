# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

project('vizzyix', 'cpp',
        version : '0.1.0',
        license : 'GPL-2.0-only',
        default_options : [
            'prefix=/usr',
            'sysconfdir=/etc',
            'cpp_std=c++17'
        ],
        meson_version : '>= 0.49',
)

qt5 = import('qt5')
qt5_dep = dependency('qt5', modules: ['Core', 'Gui', 'Svg'])

html_incs = include_directories('includes/HtmlBuilder')

# ==========================================
# Protocol Buffers Compiler
# ==========================================

protoc = find_program('protoc')

# The meson EXTRA_ARGS is only recognised as a string by itself. Not
# expanded if you prefix it with something, e.g. --wot=@EXTRA_ARGS@
proto_gen = generator(protoc,
    output: [
        '@BASENAME@.pb.cc',
        '@BASENAME@.pb.h' ],
    arguments: [
        '@EXTRA_ARGS@',
        '--cpp_out=@BUILD_DIR@',
        '@BASENAME@.proto'])

protobuf_dep = dependency('protobuf')
# ------------------------------------------

subdir('eixpb')

subdir('vizzyix')
subdir('tests')
