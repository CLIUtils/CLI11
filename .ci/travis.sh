#!/usr/bin/env sh
set -evx

mkdir build || true
cd build
cmake .. -DCLI_SINGLE_FILE_TESTS=ON
cmake --build .
ctest --output-on-failure
