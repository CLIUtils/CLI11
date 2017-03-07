# 
#
# Downloads GTest and provides a helper macro to add tests. Add make check, as well, which
# gives output on failed tests without having to set an environment variable.
#
#
set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

include(DownloadProject)
download_project(PROJ                coverage
                 GIT_REPOSITORY      https://github.com/JoakimSoderberg/coveralls-cmake.git
                 GIT_TAG             e37d5b8674dab235185b07ad9208c88d84f81823
                 UPDATE_DISCONNECTED 1
                 QUIET
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    ${coverage_SOURCE_DIR}/caake)

include(Coveralls)
coveralls_turn_on_coverage()
