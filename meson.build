# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

project('vizzyix', 'cpp',
        version : '0.5.1',
        license : 'GPL-2.0-only',
        default_options : [
            'prefix=/usr',
            'sysconfdir=/etc',
            'cpp_std=c++17'
        ],
        meson_version : '>= 1.4.1',
)

qt = import('qt6')
qt_dep = dependency('qt6', modules: ['Core', 'Gui', 'Svg', 'Widgets', 'SvgWidgets', ])

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
subdir('pbtesting')
subdir('vizzyix')
subdir('tests')
subdir('scripts')


# A way to include files into IDE project
custom_target('zz_toplevel_files',
    input: [
        '.clang-format',
        '.clang-format.license',
        'AUTHORS',
        'AUTHORS.license',
        'ChangeLog',
        'ChangeLog.license',
        'README',
    ],
    output: 'nuffink',
    command: 'ls',
)
