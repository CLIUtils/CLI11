#!/usr/bin/env bash

echo -en "travis_fold:start:script.build\\r"
echo "Building with tidy on..."
set -evx

mkdir -p build-tidy
cd build-tidy
CXX_FLAGS="-Werror -Wcast-align -Wfloat-equal -Wimplicit-atomic-properties -Wmissing-declarations -Woverlength-strings -Wshadow -Wstrict-selector-match -Wundeclared-selector -Wunreachable-code -std=c++11" cmake .. -DCLANG_TIDY_FIX=ON
cmake --build .

set -evx
echo -en "travis_fold:end:script.build\\r"
echo -en "travis_fold:start:script.compare\\r"
echo "Checking git diff..."
set -evx

git diff --exit-code --color

set +evx
echo -en "travis_fold:end:script.compare\\r"
