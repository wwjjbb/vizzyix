# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

# TODO: The proto file/path names should come from config build options
proto_generated = proto_gen.process(
    '/usr/share/eix/eix.proto',
    extra_args: '--proto_path=/usr/share/eix')

local_eixpb_lib = static_library('eixpb', proto_generated)

eixpb_dep = declare_dependency(
    link_with: local_eixpb_lib,
    include_directories: local_eixpb_lib.private_dir_include())

# A way to include files into IDE project
custom_target('zz_eixpb_files',
    input: [
        'eixpb.pro',
        'proto_compile.pri',
        'protobuf.pri',
        'readme.txt',
    ],
    output: 'nuffink',
    command: 'ls',
)
