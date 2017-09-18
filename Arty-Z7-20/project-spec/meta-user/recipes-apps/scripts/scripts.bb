#
# This file is the scripts recipe.
#

SUMMARY = "Simple scripts application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
FILES_${PN} += "${base_libdir}"
FILES_${PN}-dbg += "${base_libdir}/.debug"
FILES_SOLIBSDEV = ""
FILES_${PN} += "/usr/*"
SRC_URI = "file://Makefile \
	file://libvrc.so \
	file://libvrc.h \
		  "

S = "${WORKDIR}"

do_compile() {
	oe_runmake
}

do_install() {
	install -d ${D}/usr/lib
	install -d ${D}/usr/include
	install -m 0755 libvrc.so ${D}/usr/lib/libvrc.so
	install -m 0755 libvrc.h ${D}/usr/include/libvrc.h
}

do_install_append() {
	install -d ${D}${base_libdir}
	install -m 0755 libvrc.so ${D}${base_libdir}
	install -d ${D}/usr/include
	install -m 0755 libvrc.h ${D}/usr/include
	install -m 0755 libvrc.h ${D}${base_libdir}
}
