#!/bin/bash
# AF, HGW, 21.Sep.2023
# Build all Eclipse build configurations
# start from 3DNature/Amiga

set -e # exit on error

if [ -z "$1" ]; then
  # Wenn kein Parameter uebergeben wurde, Ermittle die Anzahl CPU-cores mit lscpu
  CORES=$(lscpu | awk '/^CPU\(s\):/ {print $2}')
else
  # Wenn ein Parameter uebergeben wurde, verwende diesen Wert
  CORES=$1
fi
printf "Using %d CPU cores.\n\n" "$CORES"


STARTDIR=$(pwd)
for BUILDCONFIG in $(find . -name "makefile" -exec dirname {} \;); do
	echo "******************************"
	echo Making $BUILDCONFIG
        echo "******************************"
	cd $BUILDCONFIG
        make clean
	make all -j $CORES
	cd $STARTDIR
done
