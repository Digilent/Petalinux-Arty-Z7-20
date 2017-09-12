# Petalinux-Arty-Z7-20

This guide will walk you through some basic steps to get you booted into Linux and rebuild the Petalinux project. After completing it, you should refer
to the Petalinux Reference Guide (UG1144) from Xilinx to learn how to do more useful things with the Petalinux toolset. Also, refer to the Known Issues section at the end of this document for a list of problems you may encounter and work arounds.

This guide assumes you are using a Linux host supported by the Petalinux tools, and have already installed the Petalinux tools to /opt/pkg/petalinux/. Digilent recommends using Ubuntu 16.04.1 LTS, as this is what we are most familiar with.

# Source the petalinux tools and generate project

First source the Petalinux environment settings by opening a terminal and running:

```
source /opt/pkg/petalinux/settings.sh
```

If you have obtained the project source directly from github, then you should simply _cd_ into the Petalinux project directory. If you have downloaded the 
.bsp, then you must first run the following command to create a new project.

```
petalinux-create -t project -s <path to .bsp file>
```

This will create a new petalinux project in your current working directory, which you should then _cd_ into.

# To run pre-built image from SD

Format an SD card with two partitions: The first should be at least 500 MB and be FAT formatted. The second needs to be at least 1.5 GB (3 GB is preferred) and can be formatted as ext4, or whatever filesystem is convenient. The second partition will be overwritten, so don't
put anything on it that you don't want to lose.

Copy _pre-built/linux/images/BOOT.BIN_ and _pre-built/linux/images/image.ub_ to the first partition of your SD card.

Identify the /dev/ node for the second partition of your SD card using _lsblk_ at the command line. It will likely take the form of /dev/sdX2, where X is a a,b,c,etc.. Then run the following command to copy the filesystem to the second partition:

### Warning! If you use the wrong /dev/ node in the following command, you will overwrite your computer's file system. BE CAREFUL

```
sudo umount /dev/sdX2
sudo dd if=pre-built/linux/images/rootfs.ext4 of=/dev/sdX2
sync
```

Eject the SD card from your computer, then do the following:

* Insert the microSD into the Arty Z7
* Attach a power source and select it with JP5
* If not already done to provide power, attach a microUSB cable between the computer and the Arty Z7
* Open a terminal program (such as minicom) and connect to the Arty Z7 with 115200/8/N/1 settings (and no Hardware flow control). The Arty Z7 typically shows up as /dev/ttyUSB1
* Optionally attach the Arty Z7 to a network using ethernet or an HDMI monitor.
* Press the PORB button to restart the Arty Z7. You should see the boot process at the terminal and eventually a root prompt.


# To run pre-built image from JTAG

TODO

# To build:

```
petalinux-config --oldconfig
petalinux-build
petalinux-package --boot --force --fsbl images/linux/zynq_fsbl.elf --fpga images/linux/Arty_Z7_20_wrapper.bit --u-boot
```

# To boot the newly built files from SD: 

Follow the same steps as done with the pre-built files, except use the files found _images/linux_.

# To prepare for release (sd boot):

```
petalinux-package --prebuilt --clean --fpga images/linux/Arty_Z7_20_wrapper.bit -a images/linux/image.ub:images/image.ub -a images/linux/rootfs.ext4:images/rootfs.ext4
petalinux-build -x distclean
petalinux-build -x mrproper
petalinux-package --bsp --force --output ../releases/Petalinux-Arty-Z7-20-SDSoC-20XX.X-X.bsp -p ./
```
Remove TMPDIR setting from project-spec/configs/config (this is done automatically for bsp project).
```
cd ..
git status # to double-check
git add .
git commit
git push
```
Finally, open a browser and go to github to push your .bsp as a release.

# To prepare for release (initramfs):
# Note: image.ub must be less than 100 MB, or github will break and the image likely won't work

```
petalinux-package --prebuilt --clean --fpga images/linux/Arty_Z7_20_wrapper.bit -a images/linux/image.ub:images/image.ub
petalinux-build -x distclean
petalinux-build -x mrproper
petalinux-package --bsp --force --output ../releases/Petalinux-Arty-Z7-20-SDSoC-2017.1-1.bsp -p ./
```
Remove TMPDIR setting from project-spec/configs/config (this is done automatically for bsp file)
```
cd ..
```
git commit and push

# TODO:
 

# Known Issues:


