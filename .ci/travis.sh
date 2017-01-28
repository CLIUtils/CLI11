#!/usr/bin/env sh
set -evx
env | sort

mkdir build || true
cd build
cmake ..
make
CTEST_OUTPUT_ON_FAILURE=1 make test
