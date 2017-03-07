#!/usr/bin/env sh
set -evx

mkdir build || true
cd build
cmake .. -DCLI_SINGLE_FILE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j2
ctest --output-on-failure
