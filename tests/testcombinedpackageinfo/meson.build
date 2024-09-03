# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

moc_files_cpi = qt.preprocess(
    moc_sources: 'tst_testcombinedpackageinfo.cpp',
    dependencies: qt_dep)

test_files_cpi = [
    'tst_testcombinedpackageinfo.cpp',
    vizzyix_sdir / 'combinedpackageinfo.cpp']

test_combinedpackageinfo = executable(
    'combinedpackageinfo',
    moc_files_cpi,
    test_files_cpi,
    dependencies: [qt_dep, protobuf_dep, qt_test_dep],
    include_directories: vixxyix_incs)

test('CombinedPackageInfo', test_combinedpackageinfo)

