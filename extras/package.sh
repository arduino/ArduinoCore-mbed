#!/bin/bash

# The scope of this file is splitting the build into self consistent packages for distribution
echo "========== Configuration ==============="

source $1

echo $VERSION
echo $FLAVOUR
echo $VARIANTS
echo $FQBNS

# Remove mbed folder content
rm -rf cores/arduino/mbed/*

# Remove libraries not in $LIBRARIES list
if [ x$FLAVOUR != x ]; then

mkdir _libraries
cd libraries
for library in $LIBRARIES; do
mv $library ../_libraries
done
cd ..
rm -rf libraries
mv _libraries libraries

# Remove variants not in $VARIANTS list
mkdir _variants
cd variants
for variant in $VARIANTS; do
mv $variant ../_variants
done
cd ..
rm -rf variants
mv _variants variants

# Remove fqbns not in $FQBNS list
touch _boards.txt
# Save all menus (will not be displayed if unused)
cat boards.txt | grep "^menu\." >> _boards.txt
for board in $FQBNS; do
cat boards.txt | grep "$board\." >> _boards.txt
done
mv _boards.txt boards.txt

fi

#Recompile mbed core, applying patches on origin/latest
set +e
./mbed-os-to-arduino -b origin/latest -a NOPE:NOPE
set -e
for variant in $VARIANTS; do
./mbed-os-to-arduino $variant:$variant
done

# Remove bootloaders not in $BOOTLOADERS list
mkdir _bootloaders
cd bootloaders
for bootloaders in $BOOTLOADERS; do
mv $bootloaders ../_bootloaders
done
cd ..
rm -rf bootloaders
mv _bootloaders bootloaders

#Patch title in platform.txt
sed -i "s/Arduino Mbed OS Boards/Arduino Mbed OS ${FLAVOUR^} Boards/g" platform.txt
sed -i "s/9.9.9/$VERSION/g" platform.txt

BASE_FOLDER=`basename $PWD`

#Package! (remove .git, patches folders)
cd ..
tar --exclude='*.git*' --exclude='*patches*' -cjhf ArduinoCore-mbed-$FLAVOUR-$VERSION.tar.bz2 $BASE_FOLDER

# Create JSON file for the package
FILENAME=ArduinoCore-mbed-$FLAVOUR-$VERSION.tar.bz2
CHKSUM=`sha256sum $FILENAME | awk '{ print $1 }'`
SIZE=`wc -c $FILENAME | awk '{ print $1 }'`
FLAVOUR_TAG=${FLAVOUR^^}_
FLAVOUR_NAME=${FLAVOUR,,}
VERSION_NAME=${VERSION//./-}

cat ArduinoCore-mbed/extras/json/mbed_$FLAVOUR_NAME-tag.template.json |
sed "s/%%VERSION%%/${VERSION}/" |
sed "s/%%${FLAVOUR_TAG}FILENAME%%/${FILENAME}/" |
sed "s/%%${FLAVOUR_TAG}CHECKSUM%%/${CHKSUM}/" |
sed "s/%%${FLAVOUR_TAG}SIZE%%/${SIZE}/" > mbed_$FLAVOUR_NAME-$VERSION_NAME.json

cd -
