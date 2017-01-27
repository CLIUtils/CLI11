include(AddGoogletest)

add_executable(CLITest CLITest.cpp ${headers})
add_gtest(CLITest)

add_executable(SmallTest SmallTest.cpp ${headers})
add_gtest(SmallTest)
