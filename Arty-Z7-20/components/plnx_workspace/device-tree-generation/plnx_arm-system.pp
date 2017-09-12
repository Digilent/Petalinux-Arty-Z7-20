# 1 "/home/digilent/sam_work/git/sbobrowicz/Petalinux-Arty-Z7-20/Arty-Z7-20/build/../components/plnx_workspace/device-tree-generation/system-top.dts"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/digilent/sam_work/git/sbobrowicz/Petalinux-Arty-Z7-20/Arty-Z7-20/build/../components/plnx_workspace/device-tree-generation/system-top.dts"







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
# 1 "/home/digilent/sam_work/git/sbobrowicz/Petalinux-Arty-Z7-20/Arty-Z7-20/build/tmp/work/plnx_arm-xilinx-linux-gnueabi/device-tree-generation/xilinx+gitAUTOINC+94fc615234-r0/system-user.dtsi" 1
/include/ "system-conf.dtsi"
# 1 "/home/digilent/sam_work/git/sbobrowicz/Petalinux-Arty-Z7-20/Arty-Z7-20/build/tmp/work-shared/plnx_arm/kernel-source/include/dt-bindings/gpio/gpio.h" 1
# 3 "/home/digilent/sam_work/git/sbobrowicz/Petalinux-Arty-Z7-20/Arty-Z7-20/build/tmp/work/plnx_arm-xilinx-linux-gnueabi/device-tree-generation/xilinx+gitAUTOINC+94fc615234-r0/system-user.dtsi" 2
/ {
 model = "Zynq ARTY Z7 Development Board";
 compatible = "digilent,zynq-artyz7", "xlnx,zynq-7000";

 xlnk {
  compatible = "xlnx,xlnk-1.0";
 };

 chosen {
  bootargs = "console=ttyPS0,115200 earlyprintk uio_pdrv_genirq.of_id=generic-uio quiet root=/dev/mmcblk0p2 rw rootwait";
 };

 aliases {
  serial0 = &uart0;
  serial1 = &uart1;
  ethernet0 = &gem0;
  spi0 = &qspi;
 };

 usb_phy0: usb_phy@0 {
  compatible = "ulpi-phy";
  #phy-cells = <0>;
  reg = <0xe0002000 0x1000>;
  view-port = <0x0170>;
  drv-vbus;
 };

 pwm: pwm@0 {
  compatible = "pwm-gpio";
  #pwm-cells = <3>;
  pwm-gpios =
   <&gpio0 54 1>,
   <&gpio0 55 1>,
   <&gpio0 56 1>,
   <&gpio0 57 1>,
   <&gpio0 58 1>,
   <&gpio0 59 1>;
 };
};

&amba_pl {
 encoder_0: digilent_encoder {
  compatible = "digilent,drm-encoder";
  digilent,fmax = <110000>;
  digilent,edid-i2c = <&i2c0>;
 };

 xilinx_drm {
  compatible = "xlnx,drm";
  xlnx,vtc = <&v_tc_0>;
  xlnx,connector-type = "HDMIA";
  xlnx,encoder-slave = <&encoder_0>;
  clocks = <&axi_dynclk_0>;
  planes {
   xlnx,pixel-format = "rgb888";
   plane0 {
    dmas = <&axi_vdma_0 0>;
    dma-names = "dma0";
   };
  };
 };
};


&axi_dynclk_0 {
 compatible = "digilent,axi-dynclk";
 #clock-cells = <0>;
 clocks = <&clkc 15>;
};

&axi_vdma_0 {
 dma-ranges = <0x00000000 0x00000000 0x20000000>;
 xlnx,num-fstores = <0x1>;
};

&v_tc_0 {
 compatible = "xlnx,v-tc-5.01.a";
};

&v_tc_1 {
 compatible = "generic-uio";
};
&axi_vdma_1 {
 compatible = "generic-uio";
};
&axi_gpio_video {
 compatible = "generic-uio";
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
# 31 "/home/digilent/sam_work/git/sbobrowicz/Petalinux-Arty-Z7-20/Arty-Z7-20/build/../components/plnx_workspace/device-tree-generation/system-top.dts" 2
