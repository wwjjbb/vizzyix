# Copyright 1999-2022 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit desktop meson

DESCRIPTION="A portage package browser"
HOMEPAGE="https://github.com/wwjjbb/vizzyix"
SRC_URI="https://github.com/wwjjbb/vizzyix/archive/v${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~mips ~sparc ~x86 ~amd64"
IUSE=""

RESTRICT="mirror"

src_install() {
	meson_src_install

	newicon vizzyix/images/eyeball.png ${PN}.png
	make_desktop_entry vizzyix Vizzyix ${PN} System

	dodoc -r LICENSES
	dodoc AUTHORS README ChangeLog
}
