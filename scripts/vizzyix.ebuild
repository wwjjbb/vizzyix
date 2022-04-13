# Copyright 1999-2020 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

inherit desktop meson

DESCRIPTION="A portage package browser"
HOMEPAGE="http://www.w-j-b.com"
SRC_URI="http://wjbsvr/distfiles/${P}.tar.xz"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~mips ~sparc ~x86 ~amd64"
IUSE=""

RESTRICT="mirror"

src_install() {
	meson_src_install

	newicon vizzyix/images/eyeball.png ${PN}.png
	make_desktop_entry vizzyix Vizzyix
	#with_desktop_entry=1

	dodoc -r LICENSES
	dodoc AUTHORS README ChangeLog
}
