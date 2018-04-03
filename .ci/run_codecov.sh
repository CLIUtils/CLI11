#!/usr/bin/env bash

echo -en "travis_fold:start:script.build\\r"
echo "Building..."
set -evx

cd ${TRAVIS_BUILD_DIR}
mkdir -p build
cd build
cmake .. -DCLI11_SINGLE_FILE_TESTS=OFF -DCLI11_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Coverage
cmake --build . -- -j2
cmake --build . --target CLI11_coverage

set +evx
echo -en "travis_fold:end:script.build\\r"
echo -en "travis_fold:start:script.lcov\\r"
echo "Capturing and uploading LCov..."
set -evx

lcov --directory . --capture --output-file coverage.info # capture coverage info
lcov --remove coverage.info '*/tests/*' '*/examples/*' '*gtest*' '*gmock*' '/usr/*' --output-file coverage.info # filter out system
lcov --list coverage.info #debug info
# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"

set +evx
echo -en "travis_fold:end:script.lcov\\r"
