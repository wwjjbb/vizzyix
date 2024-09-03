# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

version_data = configuration_data()
version_data.set('version', meson.project_version() )
configure_file(
    input: 'version.h.in',
    output: 'version.h',
    configuration: version_data)

vizzyix_src = [
    'aboutdialog.cpp',
    'applicationdata.cpp',
    'categorytreeitem.cpp',
    'categorytreemodel.cpp',
    'combinedpackageinfo.cpp',
    'combinedpackagelist.cpp',
    'eixprotohelper.cpp',
    'main.cpp',
    'mainwindow.cpp',
    'packagereportitem.cpp',
    'packagereportmodel.cpp',
    'repositoryindex.cpp',
    'searchboxvalidator.cpp'
    ]

vizzyix_hdr = [
    'categorytreeitem.h',
    'combinedpackageinfo.h',
    'combinedpackagelist.h',
    'customhtmlelements.h',
    'eixprotohelper.h',
    'localexceptions.h',
    'packagereportitem.h',
    'repositoryindex.h',
    'searchboxvalidator.h'
    ]

vizzyix_moc_hdr = [
    'aboutdialog.h',
    'applicationdata.h',
    'categorytreemodel.h',
    'mainwindow.h',
    'packagereportmodel.h',
    'searchboxvalidator.h'
    ]

vizzyix_ui = [
    'aboutdialog.ui',
    'mainwindow.ui',
    ]

moc_files = qt.preprocess(
    moc_headers: vizzyix_moc_hdr,
    ui_files: vizzyix_ui,
    qresources: 'resources.qrc',
    dependencies: qt_dep
    )

vizzyix_sdir = meson.current_source_dir()
vixxyix_incs = include_directories('.')

executable('vizzyix',
    vizzyix_src,
    moc_files,
    dependencies: [qt_dep, protobuf_dep, eixpb_dep],
    include_directories: [ html_incs ],
    extra_files: [ vizzyix_hdr,
                   vizzyix_moc_hdr,
                   vizzyix_ui,
                   'version.h.in',
                   ],
    install: true)
