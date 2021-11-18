#Get version from git(hub) tag
export VERSION="2.6.1"

FLAVOURS=`ls *.variables`

cp extras/package_index.json.NewTag.template /tmp/json

for flavour in $FLAVOURS; do

# Hack, clean everything from known positions and restart
rm -rf /tmp/mbed-os-program
git reset --hard
git clean -dxf

# Clone ArduinoCore-api in parent folder and create symlinks
git clone git@github.com:arduino/ArduinoCore-API.git ../api/
ln -s ../../../api/api cores/arduino/api
./package.sh $flavour

echo "Creating json"

source /tmp/env
CHKSUM=`sha256sum ../$FILENAME | awk '{ print $1 }'`
SIZE=`wc -c ../$FILENAME | awk '{ print $1 }'`
source ${flavour}

FLAVOUR=${FLAVOUR^^}

if [ x$FLAVOUR != x ]; then
FLAVOUR=${FLAVOUR}_
fi

echo FILENAME=$FILENAME
echo CHKSUM=$CHKSUM
echo SIZE=$SIZE
echo VERSION=$VERSION
echo FLAVOUR=$FLAVOUR

cat /tmp/json |
sed "s/%%VERSION%%/${VERSION}/" |
sed "s/%%${FLAVOUR}FILENAME%%/${FILENAME}/" |
sed "s/%%${FLAVOUR}CHECKSUM%%/${CHKSUM}/" |
sed "s/%%${FLAVOUR}SIZE%%/${SIZE}/" > /tmp/json2
mv /tmp/json2 /tmp/json

done

cp /tmp/json package_${CORE_NAME}_${VERSION}_index.json
