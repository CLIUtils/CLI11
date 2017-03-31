include(AddGoogletest)

set(CLI_TESTS
    HelpersTest
    IniTest
    SimpleTest
    AppTest
    CreationTest
    SubcommandTest
    HelpTest
    NewParseTest
    )

set(CLI_SINGLE_TESTS
    TimerTest
    )

# Only affects current directory, so safe
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(T ${CLI_TESTS})

    add_executable(${T} ${T}.cpp ${CLI_headers})
    target_link_libraries(${T} PUBLIC CLI11)
    add_gtest(${T})

    if(CLI_SINGLE_FILE AND CLI_SINGLE_FILE_TESTS)
        add_executable(${T}_Single ${T}.cpp)
        target_link_libraries(${T}_Single PUBLIC CLI11_SINGLE)
        add_gtest(${T}_Single)
    endif()

endforeach()

foreach(T ${CLI_SINGLE_TESTS})

    add_executable(${T} ${T}.cpp ${CLI_headers})
    target_link_libraries(${T} PUBLIC CLI11)
    add_gtest(${T})

endforeach()


# Link test (build error if inlines missing)
add_library(link_test_1 link_test_1.cpp)
target_link_libraries(link_test_1 PUBLIC CLI11)
add_executable(link_test_2 link_test_2.cpp)
target_link_libraries(link_test_2 PUBLIC CLI11 link_test_1)
add_gtest(link_test_2)
