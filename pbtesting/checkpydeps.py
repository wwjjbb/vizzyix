#!/usr/bin/python

# SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
#
# SPDX-License-Identifier: GPL-2.0-only

# Find packages which:
#  - require python but not 3.8 or 3.9
#  - stable
#  - current (best install)

import eix_pb2
import re

EVERYTHING = False

def installed_marker(ver):
    return "  <<<<<<<<<<<" if ver.HasField("installed") else ""

def is_stable(ver):
    if ver.HasField("system_key_flags"):
        if eix_pb2.KeyFlags.ARCHSTABLE in ver.system_key_flags.key_flag:
            return True
    if ver.HasField("local_key_flags"):
        if eix_pb2.KeyFlags.ARCHSTABLE in ver.local_key_flags.key_flag:
            return True
    return False

# True if care about this version, else false
def care_about(ver):
    # Don't care if not gentoo overlay
    if ver.HasField("repository"):
        if ver.repository.repository != "":
            return False

    if not is_stable(ver):
        return False

    return True

def check_python_target(reqd_use):
    which = set(re.findall(r"\bpython_targets_python(\d_\d)\b", reqd_use))
    return " - TGT x3.8" if "3_8" not in which else None

def check_python_single_target(reqd_use):
    which = set(re.findall(r"\bpython_single_target_python(\d_\d)\b", reqd_use))
    return " - SGL x3.8" if "3_8" not in which else None

# Return true if problem with python 3.8
def checkVersion(ver):
    result = None
    reqd = ver.required_use
    if "python_targets_" in reqd:
        result = check_python_target(reqd)
    elif "python_single_target_" in reqd:
        result = check_python_single_target(reqd)
    return result

collection = eix_pb2.Collection()
with open("eix-current.pb", "rb") as f:
    collection.ParseFromString(f.read())

if EVERYTHING:
    print(collection)
else:
    for cat in collection.category:
        for pkg in cat.package:
            # EIX lists versions in ascending order, so finding highest stable is doable
            # as long as try not to think about slots...
            msg = None
            for ver in pkg.version:
                if care_about(ver):
                    splat = checkVersion(ver)
                    if splat != None:
                        msg = "{}/{}-{} {}{}".format(cat.category,pkg.name,ver.id, splat, installed_marker(ver))
                    else:
                        msg = None
            if msg != None:
                # Only print details if best stable version has problem
                print(msg)
