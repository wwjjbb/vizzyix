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

moc_files = qt5.preprocess(
    moc_headers: [
        'aboutdialog.h',
        'applicationdata.h',
        'categorytreemodel.h',
        'mainwindow.h',
        'packagereportmodel.h',
        'searchboxvalidator.h'
        ],
    ui_files: [
         'aboutdialog.ui',
         'mainwindow.ui'],
    qresources: 'resources.qrc',
    dependencies: qt5_dep
    )

vizzyix_sdir = meson.current_source_dir()
vixxyix_incs = include_directories('.')

executable('vizzyix',
    vizzyix_src,
    moc_files,
    dependencies: [qt5_dep, protobuf_dep, eixpb_dep],
    include_directories: [ html_incs ],
    install: true)
