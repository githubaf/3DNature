#!/bin/bash
# AF, HGW, 21.Sep.2023
# Build all Eclipse build configurations
# start from 3DNature/Amiga

set -e # exit on error

STARTDIR=$(pwd)
for BUILDCONFIG in $(find . -name "makefile" -exec dirname {} \;); do
	echo "******************************"
	echo Making $BUILDCONFIG
        echo "******************************"
	cd $BUILDCONFIG
	make all
	cd $STARTDIR
done
