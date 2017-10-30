#
# This file is the libpwm recipe.
#

SUMMARY = "Simple pwm library"
SECTION = "libs"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

RDEPENDS_${PN} = "glibc libuio"
DEPENDS = "glibc libuio"

SRC_URI = "file://Makefile \
	file://libpwm.c \
	file://libpwm.h \
		  "

FILES_${PN} = "${libdir}/*.so.* ${includedir}/* ${libdir}/*.so"

FILES_SOLIBSDEV = ""
FILES_${PN}-dev = ""
INSANE_SKIP_${PN} += "dev-so"
SOLIBS = ".so"


S = "${WORKDIR}"

PACKAGE_ARCH = "${MACHINE_ARCH}"
PROVIDES = "pwm"
TARGET_CC_ARCH += "${LDFLAGS}"


do_install() {
		install -d ${D}${libdir}
		oe_libinstall -so libpwm ${D}${libdir}
		
		install -d ${D}${includedir}
	   	install -m 0644 ${S}/libpwm.h ${D}${includedir}


		install -m 0644 libpwm.so ${D}${libdir}


}
