#!/bin/bash

# The scope of this file is splitting the build into self consistent packages for distribution

# First target: makers
FLAVOUR="makers"
VARIANTS=("NANO_RP2040_CONNECT ARDUINO_NANO33BLE")
FQBNS=("nanorp2040connect nano33ble")
LIBRARIES=("PDM SPI Wire MRI USBHID USBMSD ThreadDebug Scheduler")
BOOTLOADERS=("nano33ble")
VERSION="1.5.0"

# Remove mbed folder content
rm -rf cores/arduino/mbed/*
# Remove libraries not in $LIBRARIES list
mkdir _libraries
cd libraries
for library in $LIBRARIES; do
mv $library ../_libraries
done
cd ..
rm -rf libraries
mv _libraries libraries

# Remove variants not in $VARIANTS list
mkdir _variants
cd variants
for variant in $VARIANTS; do
mv $variant ../_variants
done
cd ..
rm -rf variants
mv _variants variants

# Remove fqbns not in $FQBNS list
touch _boards.txt
for board in $FQBNS; do
cat boards.txt | grep $board >> _boards.txt
done
mv _boards.txt boards.txt

#Recompile mbed core, applying patches on origin/latest
./mbed-os-to-arduino -b origin/latest -a NANO_RP2040_CONNECT:NANO_RP2040_CONNECT
./mbed-os-to-arduino ARDUINO_NANO33BLE:ARDUINO_NANO33BLE

# Remove bootloaders not in $BOOTLOADERS list
mkdir _bootloaders
cd bootloaders
for bootloaders in $BOOTLOADERS; do
mv $bootloaders ../_bootloaders
done
cd ..
rm -rf bootloaders
mv _bootloaders bootloaders

#Patch title in platform.txt
sed -i "s/Arduino Mbed OS Boards/Arduino Mbed OS $FLAVOUR Boards/g" platform.txt
sed -i 's/9.9.9/$VERSION/g' platform.txt

#Package! (remove .git, patches folders)
cd ..
tar --exclude='*.git*' --exclude='*patches*' -cjhvf ArduinoCore-mbed-$FLAVOUR-$VERSION.tar.bz2 ArduinoCore-mbed
cd -