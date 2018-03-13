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

echo "Building testing backend"

cd targets/combo/dummy
make clean
if make; then
	make install
	cd ../../..
else
	cd ../../..
	echo -e $COLOR_LIGHT_RED "[BUILD FAILED] - Dummy API" $COLOR_NC
fi

clear

echo "Building application"
make clean
if make -j 24; then
	cd CLI
else
	echo -e $COLOR_LIGHT_RED "[BUILD FAILED] - App" $COLOR_NC
fi

clear

labels=("Assign device (0)" "Add LPM rule" "Add 2 LPM rules" "Add exact rule" "Add two same rules" "Overfill table capacity" "Add rules to multiple tables" "Set default rule" "Set/unset/set default rule" "Modify rule with handle" "Modify rule with key" "Delete rule with handle" "Delete rule with key" "Delete with with handle (multiple rules)")

for i in {1..14}
do
	rm -f log.txt
	echo "Test" $i "-" ${labels[$i-1]}
	
	if ./pi_CLI_combo -c ../tests/combo/info.json >$i.out 2>&1 <../tests/combo/in/$i.txt; then
		if diff -N log.txt ../tests/combo/out/$i.txt >$i.diff 2>&1; then
			echo -e $COLOR_LIGHT_GREEN "[OK]" $COLOR_NC
			rm -f $i.diff $i.out log.txt
		else
			echo -e $COLOR_LIGHT_RED "[FAILED] - Diff failed" $COLOR_NC
		fi
	else
		echo -e $COLOR_LIGHT_RED "[FAILED] - Program failed" $COLOR_NC
	fi
done
