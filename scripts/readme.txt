SPDX-FileCopyrightText: none
SPDX-License-Identifier: CC0-1.0

To produce a new version:

1. Update the version number in the top-level meson.build
2. Commit the changes
3  cd into scripts directory
4. Run ./mkpackage
5. Copy the archive file into the localhost distfiles
8. Copy the ebuild file into app-portage/vizzyix of the local overlay
7. Run ebuild digest
