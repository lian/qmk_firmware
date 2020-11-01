#!/bin/bash
dir="$(dirname $(dirname $(dirname $(dirname $(readlink -f $0)))))"
cd $dir

firmware=".build/handwired_lian_default.hex"
device=/dev/ttyACM0

[ -f "$firmware" ] && avrdude -p atmega32u4 -c avr109 -P $device -U "flash:w:$firmware"
