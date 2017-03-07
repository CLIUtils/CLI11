cd ${TRAVIS_BUILD_DIR}

cd build
cmake .. -DCLI_SINGLE_FILE_TESTS=OFF -DCLI_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . --target CLI_coverage
ctest --output-on-failure

lcov --directory . --capture --output-file coverage.info # capture coverage info
lcov --remove coverage.info '/usr/*' --output-file coverage.info # filter out system
lcov --list coverage.info #debug info
# Uploading report to CodeCov
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports"
