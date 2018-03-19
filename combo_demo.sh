#!/bin/bash

COLOR_NC='\e[0m' # No Color
COLOR_WHITE='\e[1;37m'
COLOR_BLACK='\e[0;30m'
COLOR_BLUE='\e[0;34m'
COLOR_LIGHT_BLUE='\e[1;34m'
COLOR_GREEN='\e[0;32m'
COLOR_LIGHT_GREEN='\e[1;32m'
COLOR_CYAN='\e[0;36m'
COLOR_LIGHT_CYAN='\e[1;36m'
COLOR_RED='\e[0;31m'
COLOR_LIGHT_RED='\e[1;31m'
COLOR_PURPLE='\e[0;35m'
COLOR_LIGHT_PURPLE='\e[1;35m'
COLOR_BROWN='\e[0;33m'
COLOR_YELLOW='\e[1;33m'
COLOR_GRAY='\e[0;30m'
COLOR_LIGHT_GRAY='\e[0;37m'

export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig

if ./configure --with-proto --without-internal-rpc --without-cli; then
	echo -e $COLOR_LIGHT_GREEN "[CONFIGURE OK]" $COLOR_NC
else
	cd ../../..
	echo -e $COLOR_LIGHT_RED "[CONFIGURE FAILED]" $COLOR_NC
	exit 1
fi

echo "Building testing backend"

cd targets/combo/dummy
make clean
if make; then
	echo -e $COLOR_LIGHT_GREEN "[MAKE OK]" $COLOR_NC
	make install-local
	cd ../../..
else
	cd ../../..
	echo -e $COLOR_LIGHT_RED "[BUILD FAILED] - Dummy API" $COLOR_NC
	exit 1
fi

echo "Building application"
make clean
if make -j 24; then
	echo -e $COLOR_LIGHT_GREEN "[MAKE OK]" $COLOR_NC
else
	echo -e $COLOR_LIGHT_RED "[BUILD FAILED] - App" $COLOR_NC
	exit 1
fi


