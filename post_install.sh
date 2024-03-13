#!/usr/bin/env bash

arduino_mbed_rules () {
cat <<EOF

# Arduino Mbed bootloader mode udev rules

SUBSYSTEMS=="usb", ATTRS{idVendor}=="2e8a", TAG+="uaccess"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2341", TAG+="uaccess"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="1fc9", TAG+="uaccess"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0525", TAG+="uaccess"
EOF
}

if [ "$EUID" -ne 0 ]; then
  echo "Please run as root"
  exit
fi

arduino_mbed_rules > /etc/udev/rules.d/60-arduino-mbed.rules

# reload udev rules
echo "Reload rules..."
udevadm trigger
udevadm control --reload-rules
