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

ALTLOG=/tmp/pi_testing.log
rm -f $ALTLOG

echo "Building application"
echo "Building application" >>$ALTLOG
make clean
if make -j 24; then
	cd CLI
else
	echo -e $COLOR_LIGHT_RED "[BUILD FAILED] - App" $COLOR_NC
	echo "[BUILD FAILED] - App" >>$ALTLOG
	exit 1
fi

clear

labels=("Assign device (0)" "Add LPM rule" "Add 2 LPM rules" "Add exact rule" "Add two same rules" "Overfill table capacity" "Add rules to multiple tables" "Set default rule" "Set/unset/set default rule" "Modify rule with handle" "Modify rule with key" "Delete rule with handle" "Delete rule with key" "Delete with with handle (multiple rules)")

ret=0
for i in {1..14}
do
	rm -f /tmp/libp4dev.log
	echo "Test" $i "-" ${labels[$i-1]}
	echo "Test" $i "-" ${labels[$i-1]} >>$ALTLOG
	
	if ./pi_CLI_combo -c ../tests/combo/info.json >$i.out 2>&1 <../tests/combo/in/$i.txt; then
		if diff -N /tmp/libp4dev.log ../tests/combo/out/$i.txt >$i.diff 2>&1; then
			echo -e $COLOR_LIGHT_GREEN "[OK]" $COLOR_NC
			echo "[OK]" >>$ALTLOG
			rm -f $i.diff $i.out log.txt
		else
			echo -e $COLOR_LIGHT_RED "[FAILED] - Diff failed" $COLOR_NC
			echo "[FAILED] - Diff failed" >>$ALTLOG
			$ret=1
		fi
	else
		echo -e $COLOR_LIGHT_RED "[FAILED] - Program failed" $COLOR_NC
		echo "[FAILED] - Program failed" >>$ALTLOG
		$ret=1
	fi
done

exit $ret