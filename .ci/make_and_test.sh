#!/usr/bin/env bash
echo -en "travis_fold:start:script.build\\r"
echo "Building..."
STD=$1
shift
set -evx


mkdir -p build
cd build
cmake .. -DCLI11_SINGLE_FILE=ON -DCLI11_CXX_STD=$STD -DCLI11_SINGLE_FILE_TESTS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER_LAUNCHER=ccache $@
cmake --build . -- -j2

set +evx
echo -en "travis_fold:end:script.build\\r"
echo -en "travis_fold:start:script.test\\r"
echo "Testing..."
set -evx

ctest --output-on-failure

set +evx
echo -en "travis_fold:end:script.test\\r"
