#!/usr/bin/env sh
set -evx

 
if [ -n "$COVERALLS" ] ; then 
    export COV1="-DCLI_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug"
    export COVERALLS_SERVICE_NAME=travis-ci
fi


mkdir build || true
cd build
cmake .. -DCLI_SINGLE_FILE_TESTS=ON $COV1
cmake --build .
if [ -n "$COVERALLS" ] ; then 
    cmake --build . --target coveralls
fi
ctest --output-on-failure
