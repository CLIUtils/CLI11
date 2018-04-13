#!/bin/env sh
# (Source me)
set -evx

LCOV_URL="http://ftp.de.debian.org/debian/pool/main/l/lcov/lcov_1.13.orig.tar.gz"
cd "${DEPS_DIR}"

if [[ ! -f "${DEPS_DIR}/lcov/bin/lcov" ]] ; then
  echo "Downloading lcov"
  mkdir -p lcov
  travis_retry wget --no-check-certificate --quiet -O - "${LCOV_URL}" | tar --strip-components=1 -xz -C lcov
fi

export PATH="${DEPS_DIR}/lcov/bin:${PATH}"
cd "${TRAVIS_BUILD_DIR}"

set +evx
