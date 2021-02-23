#!/bin/bash

# Upload wrapper fro RP2040

BASE=`dirname "${BASH_SOURCE[0]}"`

cd $BASE

./elf2uf2 $1 $1.uf2

NEXT_WAIT_TIME=0
until [ $NEXT_WAIT_TIME -eq 5 ] || ./picotool info >/dev/null 2>&1 ; do
    sleep $(( NEXT_WAIT_TIME++ ))
    echo -n "."
done

set -e
./picotool info >/dev/null 2>&1
set +e

./picotool load $1.uf2

./picotool reboot >/dev/null 2>&1
