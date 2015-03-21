#!/bin/sh
echo 0 > /sys/class/vtconsole/vtcon1/bind
rmmod rp_usbdisplay
