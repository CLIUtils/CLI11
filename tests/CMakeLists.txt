include(AddGoogletest)

set(CLI_TESTS SmallTest IniTest CLITest)

foreach(T ${CLI_TESTS})

    add_executable(${T} ${T}.cpp)
    target_link_libraries(${T} PUBLIC CLI)
    add_gtest(${T})

    if(CLI_SINGLE_FILE)

        add_executable(${T}_Single ${T}.cpp)
        target_link_libraries(${T}_Single PUBLIC CLI_SINGLE)
        add_gtest(${T}_Single)

    endif()

endforeach()

