# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

moc_files_cti = qt.preprocess(
    moc_sources: 'testpackagereportitem.cpp',
    dependencies: [
        qt_dep,
      ],
    )

test_files_cti = [
    'testpackagereportitem.cpp',
    vizzyix_sdir / 'eixprotohelper.cpp',
    vizzyix_sdir / 'combinedpackageinfo.cpp',
    vizzyix_sdir / 'combinedpackagelist.cpp',
    vizzyix_sdir / 'packagereportitem.cpp']

testdata_filename = meson.project_source_root() / 'pbtesting' / 'eix.pb'

test_packagereportitem = executable(
    'testpackagereportitem',
    moc_files_cti,
    test_files_cti,
    dependencies: [
        qt_dep,
        protobuf_dep,
        qt_test_dep,
        eixpb_dep,
      ],
    include_directories: vixxyix_incs,
    cpp_args: '-DTESTDATA="' + testdata_filename + '"')

test('PackageReportItem', test_packagereportitem)

