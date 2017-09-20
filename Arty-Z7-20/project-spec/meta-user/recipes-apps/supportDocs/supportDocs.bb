#
# This file is the supportDocs recipe.
#

SUMMARY = "Simple supportDocs application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
FILES_${PN} += "${libdir} ${includedir} /home/root/* /lib/* /usr/* /usr/include/*.h"
RDEPENDS_${PN} = ""
FILES_${PN}-dbg_append = "${base_libdir}/.debug"
FILES_${PN}-dev_remove = "${FILES_SOLIBSDEV}"

SRC_URI = "file://ArtyHW.dtbo \
	file://ArtyI2Covr.bit \
	file://ArtyI2Covr.dtbo \
	file://ArtySPIovr.bit \
	file://ArtySPIovr.dtbo \
	file://ArtyZ7.c \
	file://ArtyZ7.h \
	file://artyz7demo.c \
	file://gpio-fpga-driver.c \
	file://gpio-fpga.h \
	file://libvrc.h \
	file://libvrc.so \
	file://oled2.bit \
	file://pwm-fpga-driver.c \
	file://pwm-fpga.h \
	file://spichild.dtbo \
	file://uio-user.c \
	file://uio-user.h \
	file://uioLibraryDocs/spi-fpga-driver.c \
	file://uioLibraryDocs/spi-fpga.h \
	file://uioLibraryDocs/i2c-fpga-driver.c \
	file://uioLibraryDocs/i2c-fpga.h \
	file://uioLibraryDocs/ad2test.c \
	file://uioLibraryDocs/ioxptest.c \
	file://uioLibraryDocs/oledrgbtest.c \
	file://uioLibraryDocs/pwmtest.c \
	file://uioLibraryDocs/uiotest.c \
	file://uioLibraryDocs/gpio-fpga-driver.c \
	file://uioLibraryDocs/gpio-fpga.h \
	file://uioLibraryDocs/pwm-fpga-driver.c \
	file://uioLibraryDocs/pwm-fpga.h \
	file://uioLibraryDocs/uio-user.c \
	file://uioLibraryDocs/uio-user.h \
	file://drmdemo.c \
	"

S = "${WORKDIR}"

do_install() {
	install -d ${D}/home/root
	install -d ${D}/home/root/uioLibraryDocs
	install -d ${D}/home/root/ArtyZ720lib
	install -d ${D}/usr/include
	install -d ${D}/usr/lib
	install -d ${D}/lib
	install -d ${D}/lib/firmware
	
	install -m 0755 ${S}/ArtyZ7.c ${D}/home/root/ArtyZ720lib
	install -m 0755 ${S}/ArtyZ7.h ${D}/home/root/ArtyZ720lib
	install -m 0755 ${S}/artyz7demo.c ${D}/home/root/ArtyZ720lib
	install -m 0755 ${S}/gpio-fpga-driver.c ${D}/home/root/ArtyZ720lib
	install -m 0755 ${S}/gpio-fpga.h ${D}/home/root/ArtyZ720lib
	install -m 0755 ${S}/pwm-fpga-driver.c ${D}/home/root/ArtyZ720lib
	install -m 0755 ${S}/pwm-fpga.h ${D}/home/root/ArtyZ720lib
	install -m 0755 ${S}/uio-user.c ${D}/home/root/ArtyZ720lib
	install -m 0755 ${S}/uio-user.h ${D}/home/root/ArtyZ720lib

	install -m 0755 ${S}/drmdemo.c ${D}/home/root/ArtyZ720lib

	install -m 0755 ${S}/uioLibraryDocs/* ${D}/home/root/uioLibraryDocs
	
	install -m 0755 ${S}/ArtyI2Covr.bit ${D}/lib/firmware
	install -m 0755 ${S}/ArtyHW.dtbo ${D}/lib/firmware
	install -m 0755 ${S}/ArtyI2Covr.dtbo ${D}/lib/firmware
	install -m 0755 ${S}/ArtySPIovr.bit ${D}/lib/firmware
	install -m 0755 ${S}/ArtySPIovr.dtbo ${D}/lib/firmware
	install -m 0755 ${S}/spichild.dtbo ${D}/lib/firmware
	install -m 0755 ${S}/oled2.bit ${D}/lib/firmware
	
	install -m 0755 ${S}/libvrc.h ${D}/usr/include
	install -m 0755 ${S}/libvrc.so ${D}/usr/lib
	install -m 0755 ${S}/libvrc.h ${D}/lib
}
