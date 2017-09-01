#!/usr/bin/env sh
set -evx

git ls-files -- '*.cpp' '*.hpp' | xargs clang-format -i -style=file
git diff --exit-code

mkdir build || true
cd build
cmake ..
cmake --build .

set +evx
