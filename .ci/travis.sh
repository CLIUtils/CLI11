#!/usr/bin/env sh
set -evx
env | sort

 
if [[ "$DEPLOY_MAT" == "yes" ]] ; then 
    export COV1="-DCLI11_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug"
fi


mkdir build || true
cd build
cmake .. -DCLI_SINGLE_FILE_TESTS=ON
cmake --build .
cmake --build . --target coveralls
ctest --output-on-failure
