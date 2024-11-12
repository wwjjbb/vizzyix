# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

moc_files_ctm = qt.preprocess(
    moc_headers: vizzyix_sdir / 'categorytreemodel.h',
    moc_sources: 'tst_testcategorytreemodel.cpp',
    dependencies: [
        qt_dep,
      ],
    )

test_files_ctm = [
    'tst_testcategorytreemodel.cpp',
    vizzyix_sdir / 'categorytreeitem.cpp',
    vizzyix_sdir / 'categorytreemodel.cpp',
    ]

test_categorytreemodel = executable(
    'testcategorytreemodel',
    moc_files_ctm,
    test_files_ctm,
    dependencies: [
        qt_dep,
        protobuf_dep,
        qt_test_dep,
        eixpb_dep,
      ],
    include_directories: vixxyix_incs)

test('CategoryTreeModel', test_categorytreemodel)

