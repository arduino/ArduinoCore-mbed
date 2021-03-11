#!/bin/bash

# The scope of this file is splitting the build into self consistent packages for distribution

# First target: makers
VARIANTS="NANO_RP2040_CONNECT ARDUINO_NANO33BLE"
FQBNS=("nanorp2040connect" "nano33ble")
LIBRARIES="PDM SPI Wire MRI USBHID USBMSD ThreadDebug Scheduler"

# Remove mbed folder content
rm -rf cores/arduino/mbed/*
# Remove libraries not in $LIBRARIES list
mkdir _libraries
cd libraries
mv $LIBRARIES ../_libraries
cd ..
rm -rf libraries
mv _libraries libraries

# Remove variants not in $VARIANTS list
mkdir _variants
cd variants
mv $VARIANTS ../_variants
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

#Package!