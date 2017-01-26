cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(googletest)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(CLITest CLITest.cpp ${headers})
target_link_libraries(CLITest gtest gtest_main)
add_test(CLITest CLITest)

add_executable(SmallTest SmallTest.cpp ${headers})
target_link_libraries(SmallTest gtest gtest_main)
add_test(SmallTest SmallTest)
