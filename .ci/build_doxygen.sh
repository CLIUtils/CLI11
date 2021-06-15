#!/bin/env sh
# (Source me)

set -evx

#DOXYGEN_URL="http://doxygen.nl/files/doxygen-1.8.17.src.tar.gz"
DOXYGEN_URL="https://github.com/doxygen/doxygen/archive/Release_1_8_15.tar.gz"
cd "${DEPS_DIR}"

if [[ ! -f "${DEPS_DIR}/doxygen/build/bin/doxygen" ]] ; then
  echo "Downloading Doxygen"
  mkdir -p doxygen
  travis_retry wget --no-check-certificate --quiet -O - "${DOXYGEN_URL}" | tar --strip-components=1 -xz -C doxygen
  cd doxygen
  mkdir -p build
  cd build
  cmake ..
  make -j2
fi

export PATH="${DEPS_DIR}/doxygen/build/bin:${PATH}"

cd "${TRAVIS_BUILD_DIR}"

set +evx
