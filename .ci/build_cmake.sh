if [ "$TRAVIS_OS_NAME" = "linux" ] ; then CMAKE_URL="https://cmake.org/files/v3.7/cmake-3.7.2-Linux-x86_64.tar.gz" ; fi
if [ "$TRAVIS_OS_NAME" = "osx" ] ; then CMAKE_URL="https://cmake.org/files/v3.7/cmake-3.7.2-Darwin-x86_64.tar.gz" ; fi
cd "${DEPS_DIR}"

if [[ ! -f "${DEPS_DIR}/cmake/bin/cmake" ]] ; then
  echo "Downloading CMake"
  mkdir -p cmake
  travis_retry wget --no-check-certificate --quiet -O - "${CMAKE_URL}" | tar --strip-components=1 -xz -C cmake
fi

export PATH="${DEPS_DIR}/cmake/bin:${PATH}"
cd "${TRAVIS_BUILD_DIR}"
