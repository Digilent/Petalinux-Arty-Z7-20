# 1 "/home/digilent/sam_work/git/digilent/Petalinux-Arty-Z7-20/Arty-Z7-20/build/../components/plnx_workspace/device-tree-generation/system-top.dts"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/digilent/sam_work/git/digilent/Petalinux-Arty-Z7-20/Arty-Z7-20/build/../components/plnx_workspace/device-tree-generation/system-top.dts"







/dts-v1/;
/include/ "zynq-7000.dtsi"
/include/ "pl.dtsi"
/include/ "pcw.dtsi"
/ {
 chosen {
  bootargs = "earlycon";
  stdout-path = "serial0:115200n8";
 };
 aliases {
  ethernet0 = &gem0;
  serial0 = &uart1;
  serial1 = &uart0;
  spi0 = &qspi;
  spi1 = &spi0;
 };
 memory {
  device_type = "memory";
  reg = <0x0 0x20000000>;
 };
 cpus {
 };
};
# 1 "/home/digilent/sam_work/git/digilent/Petalinux-Arty-Z7-20/Arty-Z7-20/build/tmp/work/plnx_arm-xilinx-linux-gnueabi/device-tree-generation/xilinx+gitAUTOINC+94fc615234-r0/system-user.dtsi" 1
/include/ "system-conf.dtsi"
/ {
 model = "Zynq ARTY Z7 Development Board";
 compatible = "digilent,zynq-artyz7", "xlnx,zynq-7000";

 aliases {
  serial0 = &uart0;
  serial1 = &uart1;
  ethernet0 = &gem0;
  spi0 = &qspi;
 };
 chosen {
  bootargs = "console=ttyPS0,115200 earlyprintk";
  stdout-path = "serial0:115200n8";
 };

 usb_phy0: usb_phy@0 {
  compatible = "ulpi-phy";
  #phy-cells = <0>;
  reg = <0xe0002000 0x1000>;
  view-port = <0x0170>;
  drv-vbus;
 };
};

&qspi {
 u-boot,dm-pre-reloc;
};

&flash0 {
 compatible = "micron,m25p80", "s25fl128s";
};

&gem0 {
 phy-handle = <&ethernet_phy>;

 ethernet_phy: ethernet-phy@0 {
 reg = <1>;
 device_type = "ethernet-phy";
 };
};

&usb0 {
 usb-phy = <&usb_phy0>;
 /delete-property/ usb-reset;
};

&xadc_wiz_0 {
 compatible = "xlnx,axi-xadc-1.00.a";
 clocks = <&clkc 12>;
 xlnx,channels {
  #address-cells = <1>;
  #size-cells = <0>;
  channel@0 {
   reg = <1>;
  };
  channel@1 {
   reg = <2>;
  };
  channel@5 {
   reg = <6>;
  };
  channel@6 {
   reg = <7>;
  };
  channel@8 {
   reg = <9>;
  };
  channel@9 {
   reg = <10>;
  };
  channel@12 {
   reg = <13>;
  };
  channel@13 {
   reg = <14>;
  };
  channel@15 {
   reg = <16>;
  };
 };
};

&sdhci0 {
 u-boot,dm-pre-reloc;
};

&uart0 {
 u-boot,dm-pre-reloc;
 port-number = <0>;
};

&uart1 {
 port-number = <1>;
};
# 31 "/home/digilent/sam_work/git/digilent/Petalinux-Arty-Z7-20/Arty-Z7-20/build/../components/plnx_workspace/device-tree-generation/system-top.dts" 2
