# Arduino Core for mbed enabled devices

The repository contains the Arduino APIs and IDE integration files targeting a generic mbed-enabled board

## Installation

Clone the repository in `$sketchbook/hardware/arduino`

```bash
mkdir -p $sketchbook/hardware/arduino
cd $sketchbook/hardware/arduino
git clone git@github.com:arduino/ArduinoCore-mbed mbed
```

Then clone https://github.com/arduino/ArduinoCore-API in a directory at your choice. Checkout `namespace_arduino` branch.

```bash
git clone git@github.com:arduino/ArduinoCore-API -b namespace_arduino
```

Remove the symlink to `api` you can find in  `$sketchbook/hardware/arduino/mbed/cores/arduino` and replace it with a symlink to `ArduinoCore-API/api`

Open Arduino IDE; you should now see three new targets under `MBED boards` label

You may also need to install ARM build tools if you see an error like the following when you attempt to build for one of the mbed-os enabled boards.
```
fork/exec /bin/arm-none-eabi-g++: no such file or directory
```
The missing ARM build tools can be installed by using the `Boards Manager` option in the Arduino IDE to add the `Arduino mbed-enabled Boards` board package.


## Adding an mbed target

Adding a target is a mostly automatic procedure that involves running https://github.com/arduino/ArduinoCore-mbed/blob/master/mbed-os-to-arduino after setting `BOARDNAME` and `ARDUINOCORE` env variables.
Actions marked as TODO must be executed manually.

**Minimum Example**: `./mbed-os-to-arduino -r /home/alex/projects/arduino/cores/mbed-os-h747 ENVIE_M7:ENVIE_M7`

### How to build a debug version of the Arduino mbed libraries
* Modify `mbed-os-to-arduino `
```diff
mbed_compile () {
-       PROFILE_FLAG=""
        if [ x"$PROFILE" != x ]; then
                PROFILE_FLAG=--profile="$ARDUINOVARIANT"/conf/profile/$PROFILE.json
                export PROFILE=-${PROFILE^^}
+       else
+               export PROFILE="-DEBUG"
+               PROFILE_FLAG="--profile=debug"
        fi
```

## Using this core as an mbed library

You can use this core as a standard mbed library; all APIs are under `arduino` namespace (so they must be called like `arduino::digitalWrite()` )

The opposite is working as well; from any sketch you can call mbed APIs by prepending `mbed::` namespace.


