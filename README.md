# Arduino Core for mbed enabled devices

The repository contains the Arduino APIs and IDE integration files targeting a generic mbed-enabled board

## FAQ
### Source-Code Level Debugging
**Question**: "I want to debug my ArduinoCore-mbed based sketch using traditional debugging tools, i.e. gdb via SWD interface. However, the debugger is unable to locate the sources for all files, particular the mbed-os files."

**Answer**: This is due to the fact that we pre-compile the mbed-os code into a static library `libmbed.a`. Information on how to recompile `libmbed.a` for source code debugging can be found [here](#recompiling-libmbed-with-source-level-debug-support). The [Arduino Documentation](https://docs.arduino.cc/) also contains articles explaining how to debug via [Segger J-Link](https://docs.arduino.cc/tutorials/portenta-breakout/breakout-jlink-setup) and [Lauterbach TRACE32](https://docs.arduino.cc/tutorials/portenta-h7/lauterbach-debugger).

## Installation

Note: 

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

Create a symlink to `ArduinoCore-API/api` in `$sketchbook/hardware/arduino-git/mbed/cores/arduino`.

### Test things out

Open the Arduino IDE.

You should now see three new targets under the `MBED boards` label.

*This procedure does not automatically install the required ARM compiler toolchain.*

If the toolchain is missing, you'll see errors like this when you try to build for an mbed-os enabled board.:

```
fork/exec /bin/arm-none-eabi-g++: no such file or directory
```
To install ARM build tools, use the `Boards Manager` option in the Arduino IDE to add the `Arduino mbed-enabled Boards` package.

## mbed-os-to-arduino script

The backbone of the packaging process is the https://github.com/arduino/ArduinoCore-mbed/blob/master/mbed-os-to-arduino script. It basically compiles a blank Mbed OS project for any supported target board, recovering the files that will be needed at compile time and copying them to the right location. This script is compatible only with Linux. If you are using macOS, use the https://github.com/arduino/ArduinoCore-mbed/blob/master/mbed-os-to-arduino-macos script.

It can be used for a variety of tasks including:

### Recompiling libmbed with source level debug support

```
cd $sketchbook/hardware/arduino-git/mbed
./mbed-os-to-arduino -a -g PORTENTA_H7_M7:PORTENTA_H7_M7
```

In this case `-a` applies all the patches from `patches` folder into a mainline `mbed-os` tree, and `-g` restores the debug info.

### Selecting a different optimization profile

```
cd $sketchbook/hardware/arduino-git/mbed
PROFILE=release ./mbed-os-to-arduino -a NANO_RP2040_CONNECT:NANO_RP2040_CONNECT
```

The `PROFILE` environment variable tunes the compilation profiles (defaults to `DEVELOP`). Other available profiles are `DEBUG` and `RELEASE`.

### Selecting a different Mbed OS tree

```
cd $sketchbook/hardware/arduino-git/mbed
./mbed-os-to-arduino -r /path/to/my/mbed-os-fork NICLA_VISION:NICLA_VISION
```

`-r` flag allows using a custom `mbed-os` fork in place of the mainline one; useful during new target development.

### Adding a new target ([core variant](https://arduino.github.io/arduino-cli/latest/platform-specification/#core-variants))

Adding a target is a mostly automatic procedure.

For boards already supported by Mbed OS, the bare minimum is:

```
cd $sketchbook/hardware/arduino-git/mbed
mkdir -p variants/$ALREADY_SUPPORTED_BOARD_NAME/{libs,conf}
./mbed-os-to-arduino $ALREADY_SUPPORTED_BOARD_NAME:$ALREADY_SUPPORTED_BOARD_NAME
# for example, to create a core for LPC546XX
# mkdir -p variants/LPC546XX/{libs,conf}
# ./mbed-os-to-arduino LPC546XX:LPC546XX
```

This will produce almost all the files needed. To complete the port, add the board specifications to [`boards.txt`](https://arduino.github.io/arduino-cli/latest/platform-specification/#boardstxt) (giving it a unique ID) and provide `pins_arduino.h` and `variants.cpp` in `variants/$ALREADY_SUPPORTED_BOARD_NAME` folder.
Feel free to take inspirations from the existing variants :)

For boards not supported by mainline Mbed OS, the same applies but you should provide the path of your Mbed OS fork

```
cd $sketchbook/hardware/arduino-git/mbed
mkdir -p variants/$BRAND_NEW_BOARD_NAME/{libs,conf}
./mbed-os-to-arduino -r /path/to/mbed-os/fork/that/supports/new/board $BRAND_NEW_BOARD_NAME:$BRAND_NEW_BOARD_NAME
```

### Customizing Mbed OS build without modifying the code

Most Mbed OS defines can be tuned using a project file called `mbed_app.json` . In case you need to tune a build you can add that file to your variant's `conf` folder. One example is https://github.com/arduino/ArduinoCore-mbed/blob/master/variants/PORTENTA_H7_M7/conf/mbed_app.json .
Providing an invalid json or replacing a non-existing property will make the build fail silently, so it's always better to validate that file with a standard Mbed OS project.


## Using this core as an mbed library

You can use this core as a standard mbed library; all APIs are under `arduino` namespace (so they must be called like `arduino::digitalWrite()` )

The opposite is working as well; from any sketch you can call mbed APIs by prepending `mbed::` namespace.

