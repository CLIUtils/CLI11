#!/usr/bin/env sh
set -evx
 
if [ -n "$COVERALLS" ] ; then 
    export COV1="-DCLI_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug -DCLI_SINGLE_FILE_TESTS=OFF"
else
    export COV1="-DCLI_SINGLE_FILE_TESTS=ON"
fi


mkdir build || true
cd build
cmake .. $COV1
cmake --build .
ctest --output-on-failure
