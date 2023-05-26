#!/usr/bin/env bash

arduino_mbed_rules () {
    echo ""
    echo "# Arduino Mbed bootloader mode udev rules"
    echo ""
cat <<EOF
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2e8a", MODE:="0666"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2341", MODE:="0666"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="1fc9", MODE:="0666"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0525", MODE:="0666"
EOF
}

if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi

arduino_mbed_rules > /etc/udev/rules.d/60-arduino-mbed.rules

# reload udev rules
echo "Reload rules..."
udevadm trigger
udevadm control --reload-rules
