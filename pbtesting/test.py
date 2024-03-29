#!/usr/bin/python

# SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
#
# SPDX-License-Identifier: CC0-1.0

# Run this to dump the contents of the eix.pb file, which is
# used by the test modules. This lets you look at the data
# which the tests are using.

import eix_pb2

EVERYTHING = True
FINDLIVEONLY = True

collection = eix_pb2.Collection()
with open("eix.pb", "rb") as f:
    collection.ParseFromString(f.read())

if EVERYTHING:
    print(collection)
elif FINDLIVEONLY:
    for cat in collection.category:
        #print(cat.category)
        for pkg in cat.package:
            #print("   ",pkg.name)
            ver = pkg.version[0]
            if ver.id[0:3] == '999':
                print(cat.category, pkg.name)

else:
    print("Length:",len(collection.category))
    for cat in collection.category:
        print(cat.category)
        for pkg in cat.package:
            print("   ",pkg.name)

