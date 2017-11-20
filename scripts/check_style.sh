#!/usr/bin/env sh
set -evx

clang-format --version

git ls-files -- '*.cpp' '*.hpp' | xargs clang-format -i -style=file

git diff --exit-code --color

set +evx
