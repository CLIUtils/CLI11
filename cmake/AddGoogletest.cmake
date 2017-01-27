# 
#
# Downloads GTest and provides a helper macro to add tests. Add make check, as well, which
# gives output on failed tests without having to set an environment variable.
#
#

include(DownloadProject)
download_project(PROJ                googletest
                 GIT_REPOSITORY      https://github.com/google/googletest.git
                 GIT_TAG             release-1.8.0
                 UPDATE_DISCONNECTED 1
                 QUIET
)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

if (CMAKE_CONFIGURATION_TYPES)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure 
        --build-config "$<CONFIGURATION>")
else()
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure)
endif()

include_directories(${gtest_SOURCE_DIR}/include)

# More modern way to do the last line, less messy but needs newish CMake:
# target_include_directories(gtest INTERFACE ${gtest_SOURCE_DIR}/include)

# Target must already exist
macro(add_gtest TESTNAME)
    target_link_libraries(${TESTNAME} gtest gtest_main)
    add_test(${TESTNAME} ${TESTNAME})
endmacro()

