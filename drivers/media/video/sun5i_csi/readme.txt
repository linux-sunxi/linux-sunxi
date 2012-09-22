===========================================

Version: V1_13

Author:  raymonxiu

Date:     2012-3-12 10:14:50

Description:

newest module list:(X = 0 or 1)
insmod sun4i_csiX.ko ccm="ov7670" i2c_addr=0x42
insmod sun4i_csiX.ko ccm="gc0308" i2c_addr=0x42
insmod sun4i_csiX.ko ccm="gt2005" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="hi704"  i2c_addr=0x60
insmod sun4i_csiX.ko ccm="sp0838" i2c_addr=0x30
insmod sun4i_csiX.ko ccm="mt9m112" i2c_addr=0xba
insmod sun4i_csiX.ko ccm="mt9m113" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="ov2655" i2c_addr=0x60
insmod sun4i_csiX.ko ccm="hi253" i2c_addr=0x40
insmod sun4i_csiX.ko ccm="gc0307" i2c_addr=0x42
insmod sun4i_csiX.ko ccm="mt9d112" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="ov5640" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="gc2015" i2c_addr=0x60
insmod sun4i_csiX.ko ccm="ov2643" i2c_addr=0x60
insmod sun4i_csiX.ko ccm="gc0329" i2c_addr=0x62

V1_13
CSI: Fix bugs and add new module gc0309 support
1) Fix gc0308 AWB recovery
2) Fix ov2643 UXGA flicker
3) Add new module gc0309

V1_12
CSI: Optimizing for CTS test and fix bug
1) Optimizing gc0308 and gt2005 for CTS test
2) Fix clock alternating bug
3) Add gc0329 module
4) Modify all msleep to mdelay

V1_11
CSI: Modify clock gating and axp_gpio_get_io bug
1) Insure the clk_enable() and clk_disable() are called in pair
2) Fix the axp_gpio_get_io() bug for pmu gpio2

V1_10
CSI: Merge modification from sun4i and fix gc0308 red color
1) Judge if csi is generating before csi_read and csi_poll
2) Add i2c adapter lock when camera power on/off and standby on/off
3) Modify standby and reset io sequence when power on and standby off
4) Add standy and reset control before power off
5) Add new camera module gc2015,ov2643 and modify deconfig
6) Modfiy the device source code to keep identical between sun4i/5i
7) Fix gc0308 red color

V1_01
CSI: Fix bugs, add new modules support and modity power/standby interface
     Modify new camera modules default menuconfig
1) Fix bugs for CTS test
2) Fix bugs for crash when insmod after rmmod
3) Add default format for csi driver
4) Modify the power on/off,stanby on/off interface
5) Fix bugs for multipex dual sensors using one csi
6) Add gc0307, mt9d112 and ov5640 modules support
7) Fix gc0308 AWB alternation bug
8) Modify new camera modules default menuconfig

V1_00
CSI:Initial version for linux 3.0.8
1) Ported from A10 V1_02


