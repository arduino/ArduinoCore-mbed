#Get version from git(hub) tag
export VERSION="4.4.1"

FLAVOURS=`find ./extras/variables/*.variables`

for flavour in $FLAVOURS; do

# Hack, clean everything from known positions and restart
rm -rf /tmp/mbed-os-program
git reset --hard
git clean -dxf

# Clone ArduinoCore-api in parent folder and create symlinks
git clone https://github.com/arduino/ArduinoCore-API.git ../api/
ln -s ../../../api/api cores/arduino/api
./extras/package.sh $flavour

done
