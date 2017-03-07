# 
#
# Downloads GTest and provides a helper macro to add tests. Add make check, as well, which
# gives output on failed tests without having to set an environment variable.
#
#
set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")

include(DownloadProject)
download_project(PROJ                googletest
                 GIT_REPOSITORY      https://github.com/google/googletest.git
                 GIT_TAG             release-1.8.0
                 UPDATE_DISCONNECTED 1
                 QUIET
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${googletest_SOURCE_DIR} ${googletest_SOURCE_DIR})

#mark_as_advanced(
#    gtest_build_samples
#    gtest_build_tests
#    gtest_disable_pthreads
#    gtest_force_shared_crt
#    gtest_hide_internal_symbols
#    BUILD_SHARED_LIBS
#)

if (CMAKE_CONFIGURATION_TYPES)
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure 
        --build-config "$<CONFIGURATION>")
else()
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} 
        --force-new-ctest-process --output-on-failure)
endif()

#include_directories(${gtest_SOURCE_DIR}/include)

# More modern way to do the last line, less messy but needs newish CMake:
# target_include_directories(gtest INTERFACE ${gtest_SOURCE_DIR}/include)

# Target must already exist
macro(add_gtest TESTNAME)
    target_link_libraries(${TESTNAME} PUBLIC gtest gmock gtest_main)
    add_test(${TESTNAME} ${TESTNAME})
endmacro()

mark_as_advanced(
gmock_build_tests
gtest_build_samples
gtest_build_tests
gtest_disable_pthreads
gtest_force_shared_crt
gtest_hide_internal_symbols
)

