#!/usr/bin/env sh
set -evx

mkdir build || true
cd build
cmake .. -DCLI_CXX_STD=11 -DCLI_SINGLE_FILE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -- -j2
ctest --output-on-failure
if [ -n "$CLI_CXX_STD" ] && [ "$CLI_CXX_STD" -ge "14" ] ; then
    cmake .. -DCLI_CXX_STD=14 -DCLI_SINGLE_FILE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
    cmake --build . -- -j2
    ctest --output-on-failure
fi
if [ -n "$CLI_CXX_STD" ] && [ "$CLI_CXX_STD" -ge "17" ] ; then
    cmake .. -DCLI_CXX_STD=17 -DCLI_SINGLE_FILE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
    cmake --build . -- -j2
    ctest --output-on-failure
fi

set +evx
