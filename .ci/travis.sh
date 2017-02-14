#!/usr/bin/env sh
set -evx
env | sort

mkdir build || true
cd build
cmake .. -DCLI_SINGLE_FILE_TESTS=ON
make
CTEST_OUTPUT_ON_FAILURE=1 make test
