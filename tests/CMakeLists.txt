cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

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

add_executable(CLITest CLITest.cpp ${headers})
target_link_libraries(CLITest gtest gtest_main)
add_test(CLITest CLITest)

add_executable(SmallTest SmallTest.cpp ${headers})
target_link_libraries(SmallTest gtest gtest_main)
add_test(SmallTest SmallTest)
