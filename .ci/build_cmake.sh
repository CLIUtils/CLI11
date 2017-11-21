CMAKE_VERSION=3.9.6
CMAKE_MVERSION=${CMAKE_VERSION%.*}
# Non Bash version:
# echo CMAKE_MVERSION=`$var | awk -F"." '{ print $1"."$2 }'`

if [ "$TRAVIS_OS_NAME" = "linux" ] ; then CMAKE_URL="https://cmake.org/files/v${CMAKE_MVERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.tar.gz" ; fi
if [ "$TRAVIS_OS_NAME" = "osx" ] ; then CMAKE_URL="https://cmake.org/files/v$CMAKE_MVERSION/cmake-$CMAKE_VERSION-Darwin-x86_64.tar.gz" ; fi
cd "${DEPS_DIR}"

if [[ ! -f "${DEPS_DIR}/cmake/bin/cmake" ]] ; then
  echo "Downloading CMake $CMAKE_VERSION"
  mkdir -p cmake
  travis_retry wget --no-check-certificate --quiet -O - "${CMAKE_URL}" | tar --strip-components=1 -xz -C cmake
fi

export PATH="${DEPS_DIR}/cmake/bin:${PATH}"
cd "${TRAVIS_BUILD_DIR}"
