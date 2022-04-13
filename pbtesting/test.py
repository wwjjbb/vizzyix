#!/usr/bin/python

# SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
#
# SPDX-License-Identifier: CC0-1.0

import eix_pb2

EVERYTHING = True

collection = eix_pb2.Collection()
with open("eix-current.pb", "rb") as f:
    collection.ParseFromString(f.read())

if EVERYTHING:
    print(collection)
else:
    print("Length:",len(collection.category))
    for cat in collection.category:
        print(cat.category)
        for pkg in cat.package:
            print("   ",pkg.name)
