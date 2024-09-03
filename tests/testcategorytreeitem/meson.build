# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

moc_files_cti = qt.preprocess(
    moc_sources: 'tst_testcategorytreeitem.cpp',
    dependencies: qt_dep)

test_files_cti = [
    'tst_testcategorytreeitem.cpp',
    vizzyix_sdir / 'categorytreeitem.cpp']

test_categorytreeitem = executable(
    'testcategorytreeitem',
    moc_files_cti,
    test_files_cti,
    dependencies: [qt_dep, protobuf_dep, qt_test_dep, eixpb_dep],
    include_directories: vixxyix_incs)

test('CategoryTreeItem', test_categorytreeitem)

