# SPDX-FileCopyrightText: 2020 Bill Binder <dxtwjb@gmail.com>
#
# SPDX-License-Identifier: CC0-1.0

Qmake and protoc are both very opionated tools, and trying to make them work
together is a nightmare.
  * Qmake has inadequate docs on creating a new 'compiler', especially about
    the file path variables available in the setting up. Maybe there are some
    path manipulation functions hiding somewhere.
  * Protoc paths. Just that really. Its happy when the generated files are
    in the same directory. And ditto for any included proto files. Otherwise
    it can make life living hell.

Meson and protoc also have problems. There is a meson test case for protc but
it does NOT have a relative path so thats a very easy case.
