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

OS="$(uname -s)"
case "$OS" in
Linux*)
  if [ "$EUID" -ne 0 ]; then
    if [ -e "${PWD}/post_install.sh" ]; then
      echo
      echo "You might need to configure permissions for uploading."
      echo "To do so, run the following command from the terminal:"
      echo "sudo \"${PWD}/post_install.sh\""
      echo
    else
      # Script was executed from another path. It is assumed this will only occur when user is executing script directly.
      # So it is not necessary to provide the command line.
      echo "Please run as root"
    fi

    exit
  fi

  arduino_mbed_rules > /etc/udev/rules.d/60-arduino-mbed.rules

  # reload udev rules
  echo "Reload rules..."
  udevadm trigger
  udevadm control --reload-rules

  ;;
esac
