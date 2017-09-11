#!/usr/bin/env sh
set -evx

mkdir build || true
cd build
cmake .. -DCLI_CXX_STD=11 -DCLI_SINGLE_FILE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j2
ctest --output-on-failure

if [ "$CLI_CXX_STD" == "14" ] ; then
    cmake .. -DCLI_CXX_STD=14 -DCLI_SINGLE_FILE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
    cmake --build . -- -j2
    ctest --output-on-failure
fi

set +evx
