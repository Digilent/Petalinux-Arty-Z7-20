# Petalinux-Arty-Z7-20

To build:

petalinux-build
petalinux-package --boot --force --fsbl images/linux/zynq_fsbl.elf --fpga images/linux/Arty_Z7_20_wrapper.bit --u-boot
petalinux-package --prebuilt --clean --fpga images/linux/Arty_Z7_20_wrapper.bit -a images/linux/rootfs.ext4.gz:images/rootfs.ext4.gz -a images/linux/image.ub:images/image.ub

