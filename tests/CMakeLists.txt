include(AddGoogletest)

add_executable(CLITest CLITest.cpp)
target_link_libraries(CLITest PUBLIC CLI)
add_gtest(CLITest)

add_executable(SmallTest SmallTest.cpp)
target_link_libraries(SmallTest PUBLIC CLI)
add_gtest(SmallTest)

if(CLI_SINGLE_FILE)

    add_executable(CLISingleTest CLITest.cpp)
    target_link_libraries(CLISingleTest PUBLIC CLI_SINGLE)
    add_gtest(CLISingleTest)

    add_executable(SmallSingleTest SmallTest.cpp)
    target_link_libraries(SmallSingleTest PUBLIC CLI_SINGLE)
    add_gtest(SmallSingleTest)

endif()
