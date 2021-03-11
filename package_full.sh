#Get version from git(hub) tag
export VERSION="1.5.0"

FLAVOURS=`ls *.variables`

for flavour in $FLAVOURS; do

# Hack, clean everything from known positions and restart
rm -rf /tmp/mbed-os-program
git reset --hard
git clean -dxf

./package.sh $flavour

done
