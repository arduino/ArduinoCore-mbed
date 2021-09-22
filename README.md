# Arduino Core for mbed enabled devices

The repository contains the Arduino APIs and IDE integration files targeting a generic mbed-enabled board

## Installation

### Clone the repository in `$sketchbook/hardware/arduino-git`

```bash
mkdir -p $sketchbook/hardware/arduino-git
cd $sketchbook/hardware/arduino-git
git clone git@github.com:arduino/ArduinoCore-mbed mbed
```

### Clone https://github.com/arduino/ArduinoCore-API into a directory of your choice.

```bash
git clone git@github.com:arduino/ArduinoCore-API
```

### Update the `api` symlink

Create a symlink to `ArduinoCore-API/api` in `$sketchbook/hardware/arduino/mbed/cores/arduino`.

### Test things out

Open the Arduino IDE.

You should now see three new targets under the `MBED boards` label.

*This procedure does not automatically install the required ARM compiler toolchain.*

If the toolchain is missing, you'll see errors like this when you try to build for an mbed-os enabled board.:

```
fork/exec /bin/arm-none-eabi-g++: no such file or directory
```
To install ARM build tools, use the `Boards Manager` option in the Arduino IDE to add the `Arduino mbed-enabled Boards` package.


## Adding an mbed target

Adding a target is a mostly automatic procedure that involves running https://github.com/arduino/ArduinoCore-mbed/blob/master/mbed-os-to-arduino after setting the `BOARDNAME` and `ARDUINOCORE` env variables.
Actions marked as TODO must be executed manually.

**Minimum Example**:
```
cd $sketchbook/hardware/arduino-git/mbed
./mbed-os-to-arduino -r /home/alex/projects/arduino/cores/mbed-os-h747 PORTENTA_H7_M7:PORTENTA_H7_M7
```

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

