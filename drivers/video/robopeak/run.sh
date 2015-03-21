#!/bin/sh
./stop.sh
cp rp_usbdisplay.ko /lib/modules/`uname -r`/
modprobe rp_usbdisplay
echo 1 > /sys/class/vtconsole/vtcon1/bind
