#!/bin/bash

cd targets/combo/dummy && make clean && make install && cd ../../..
make -j 24 && cd CLI && ./pi_CLI_combo -c ../tests/testdata/simple_router.json <clicode.txt && cd ..
