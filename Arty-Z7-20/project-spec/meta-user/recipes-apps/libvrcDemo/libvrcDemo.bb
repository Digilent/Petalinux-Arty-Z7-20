#
# This file is the libvrcDemo recipe.
#

SUMMARY = "Simple libvrcDemo application"
SECTION = "libs"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI = "file://drmdemo.c \
	   file://Makefile \
	file://libvrc.c \
	file://libvrc.h \
		  "


S = "${WORKDIR}"

PACKAGE_ARCH = "${MACHINE_ARCH}"
PROVIDES = "vrc"
TARGET_CC_ARCH += "${LDFLAGS}"


do_install() {

	    install -d ${D}${includedir}
		install -d ${D}${libdir}
		oe_libinstall -so libvrc ${D}${libdir}


	    install -d -m 0655 ${D}${includedir}
	   	install -m 0644 ${S}/libvrc.h ${D}${includedir}

#	   	${CC} ${CFLAGS} -lvrc drmdemo.c -o libvrcDemo
#	   	install -m 0755 libvrcDemo ${D}${bindir}
}


FILES_${PN} = "${libdir}/*.so.* ${includedir}/*"
FILES_${PN}-dev = "${libdir}/*.so"