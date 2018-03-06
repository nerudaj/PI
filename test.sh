#!/bin/bash

cd targets/combo/dummy && make clean && make install && cd ../../..

if make -j 24; then
	cd CLI
else
	exit 1
fi

clear
rm -f test*.log

echo "Test 01 - Add/Delete rule in LPM table"
if ./pi_CLI_combo -c ../tests/testdata/simple_router.json <combo_test01.txt >test01.log 2>&1; then
	echo "[OK]"
else
	echo "[FAILED]"
fi

echo "Test 02 - Add(2)/Delete(1) rule in LPM table"
if ./pi_CLI_combo -c ../tests/testdata/simple_router.json <combo_test02.txt >test02.log 2>&1; then
	echo "[OK]"
else
	echo "[FAILED]"
fi

echo "Test 03 - Repeatedly set/reset default rule in LPM table"
if ./pi_CLI_combo -c ../tests/testdata/simple_router.json <combo_test03.txt >test03.log 2>&1; then
	echo "[OK]"
else
	echo "[FAILED]"
fi

echo "Test 01 - Delete rule wkey in LPM table"
if ./pi_CLI_combo -c ../tests/testdata/simple_router.json <combo_test04.txt >test04.log 2>&1; then
	echo "[OK]"
else
	echo "[FAILED]"
fi

cd ..