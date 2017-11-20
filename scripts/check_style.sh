#!/usr/bin/env sh
set -evx

clang-format --version

git ls-files -- '*.cpp' '*.hpp' | xargs clang-format -i -style=file

git diff --exit-code --color

mkdir build-tidy || true
cd build-tidy
CXX_FLAGS="-Werror -Wall -Wextra -pedantic -std=c++11" cmake .. -DCLANG_TIDY_FIX=ON
cmake --build .

git diff --exit-code --color

set +evx
