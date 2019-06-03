# Arduino Core for mbed enabled devices

The repository contains the Arduino APIs and IDE integration files targeting a generic mbed-enabled board

## Installation

Clone the repository in `$sketchbook/hardware/arduino`

```bash
mkdir -p $sketchbook/hardware/arduino
cd $sketchbook/hardware/arduino
git clone git@github.com:bcmi-labs/ArduinoCore-mbed mbed
```

Then clone https://github.com/arduino/ArduinoCore-API in a directory at your choice. Checkout `namespace_arduino` branch.

```bash
git clone git@github.com:arduino/ArduinoCore-API -b namespace_arduino
```

Remove the symlink to `api` you can find in  `$sketchbook/hardware/arduino/mbed/cores/arduino` and replace it with a symlink to `ArduinoCore-API/api`

Open Arduino IDE; you should now see three new targets under `MBED boards` label

## Arduino builder modification
To handle the large number of include paths used by mbed, arduino-builder must be replaced with [this](https://github.com/arduino/arduino-builder/pull/307) reference-aware version.

## Adding an mbed target

Adding a target is a mostly automatic procedure that involves running https://github.com/bcmi-labs/ArduinoCore-mbed/blob/master/mbed-os-to-arduino after setting `BOARDNAME` and `ARDUINOCORE` env variables.
Actions marked as TODO must be executed manually.

## Using this core as an mbed library

You can use this core as a standard mbed library; all APIs are under `arduino` namespace (so they must be called like `arduino::digitalWrite()` )

The opposite is working as well; from any sketch you can call mbed APIs by prepending `mbed::` namespace.


