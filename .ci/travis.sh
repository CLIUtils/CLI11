#!/usr/bin/env sh
set -evx
 
if [ -n "$COVERALLS" ] ; then 
    export COV1="-DCLI_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug"
fi


mkdir build || true
cd build
cmake .. -DCLI_SINGLE_FILE_TESTS=ON $COV1
cmake --build .
ctest --output-on-failure
