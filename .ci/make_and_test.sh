#!/usr/bin/env sh
set -evx

mkdir -p build
cd build

cmake .. -DCLI_CXX_STD=$1 -DCLI_SINGLE_FILE_TESTS=ON -DRULE_LAUNCH_COMPILE=ccache
cmake --build . -- -j2
ctest --output-on-failure

set +evx
