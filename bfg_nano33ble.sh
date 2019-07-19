#!/bin/bash -ex

# Extract nrf core history from main repo

cd /tmp

wget https://search.maven.org/classic/remote_content?g=com.madgag\&a=bfg\&v=LATEST -O bfg.jar

mkdir ArduinoCore-nRF52-mbed.git
cd ArduinoCore-nRF52-mbed.git
git init --bare
cd ..

git clone git@github.com:bcmi-labs/ArduinoCore-mbed
cd ArduinoCore-mbed
git remote add nrf52 /tmp/ArduinoCore-nRF52-mbed.git
#don't mess with origin
git remote remove origin
rm -rf variants/ENVIE*
rm -rf variants/MBED*
sed  -i '/.*envie.*$/ d' boards.txt
sed  -i '/.*odin.*$/ d' boards.txt
rm bfg*
git commit -am "Remove unwanted targets"
git push nrf52 master
cd ..

java -jar bfg.jar --delete-folders "{ENVIE_M4,MBED_CONNECT_ODIN,MTB_UBLOX_ODIN_W2,DISCO_F429ZI,openamp,OpenAMP,adafruit-nrfutil}"  --delete-files=RPC* --no-blob-protection /tmp/ArduinoCore-nRF52-mbed.git

cd /tmp/ArduinoCore-nRF52-mbed.git
git filter-branch --prune-empty
git reflog expire --expire=now --all && git gc --prune=now --aggressive
git remote add origin git@github.com:bcmi-labs/ArduinoCore-nRF52-mbed
git push origin master
