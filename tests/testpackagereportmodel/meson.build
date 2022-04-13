# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

moc_files_ctm = qt5.preprocess(
    moc_headers: vizzyix_sdir / 'packagereportmodel.h',
    moc_sources: 'tst_testpackagereportmodel.cpp',
    dependencies: qt5_dep)

test_files_ctm = [
    'tst_testpackagereportmodel.cpp',
    vizzyix_sdir / 'eixprotohelper.cpp',
    vizzyix_sdir / 'combinedpackageinfo.cpp',
    vizzyix_sdir / 'combinedpackagelist.cpp',
    vizzyix_sdir / 'packagereportitem.cpp',
    vizzyix_sdir / 'packagereportmodel.cpp']

testdata_filename = meson.source_root() / 'pbtesting' / 'eix.pb'

test_packagereportmodel = executable(
    'testpackagereportmodel',
    moc_files_ctm,
    test_files_ctm,
    dependencies: [qt5_dep, protobuf_dep, qt5_test_dep, eixpb_dep],
    include_directories: vixxyix_incs,
    cpp_args: '-DTESTDATA="' + testdata_filename + '"')

test('PackageReportModel', test_packagereportmodel)

