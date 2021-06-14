#!/bin/bash

mbed config root .
mbed deploy
mbed target NANO_RP2040_CONNECT
mbed toolchain GCC_ARM
mbed compile
set -e
xxd -i BUILD/NANO_RP2040_CONNECT/GCC_ARM/extra_application.bin > ../src/rp2040.h
set +e
#remove last 2 lines
sed -i '$d' ../src/rp2040.h
sed -i '$d' ../src/rp2040.h
#remove first line
sed -i '1d' ../src/rp2040.h