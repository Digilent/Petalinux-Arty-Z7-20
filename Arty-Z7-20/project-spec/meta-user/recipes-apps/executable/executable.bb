#
# This file is the executable recipe.
#

SUMMARY = "Simple executable application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
INSANE_SKIP_${PN} = "already-stripped"

SRC_URI = "file://ArtyZ720demo \
	file://bitstream.sh \
	file://ioxptest \
	file://oledrgb \
	file://overlay.sh \
	"

S = "${WORKDIR}"

do_install() {
	     install -d ${D}/${bindir}
	     install -m 0755 ${S}/ArtyZ720demo ${D}/${bindir}
	install -m 0755 ${S}/bitstream.sh ${D}/${bindir}
	install -m 0755 ${S}/ioxptest ${D}/${bindir}
	install -m 0755 ${S}/oledrgb ${D}/${bindir}
	install -m 0755 ${S}/overlay.sh ${D}/${bindir}
}
