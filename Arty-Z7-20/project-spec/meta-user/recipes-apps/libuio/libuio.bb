#
# This file is the libuio recipe.
#

SUMMARY = "Simple libvrc library"
SECTION = "libs"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

RDEPENDS_${PN} = "glibc"
DEPENDS = "glibc"

SRC_URI = "file://Makefile \
	file://libuio.c \
	file://libuio.h \
		  "


FILES_${PN} = "${libdir}/*.so.* ${includedir}/* ${libdir}/*.so"

FILES_SOLIBSDEV = ""
FILES_${PN}-dev = ""
INSANE_SKIP_${PN} += "dev-so"
SOLIBS = ".so"


S = "${WORKDIR}"

PACKAGE_ARCH = "${MACHINE_ARCH}"
PROVIDES = "uio"
TARGET_CC_ARCH += "${LDFLAGS}"


do_install() {
		install -d ${D}${libdir}
		oe_libinstall -so libuio ${D}${libdir}
		
		install -d ${D}${includedir}
	   	install -m 0644 ${S}/libuio.h ${D}${includedir}

		install -m 0644 libuio.so ${D}${libdir}

}

