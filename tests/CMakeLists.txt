set(GOOGLE_TEST_INDIVIDUAL OFF)
include(AddGoogletest)

set(CLI11_TESTS
    HelpersTest
    IniTest
    SimpleTest
    AppTest
    CreationTest
    SubcommandTest
    HelpTest
    NewParseTest
    OptionalTest
    )

set(CLI11_MULTIONLY_TESTS
    TimerTest
    )

# Only affects current directory, so safe
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(T ${CLI11_TESTS})

    add_executable(${T} ${T}.cpp ${CLI11_headers})
    target_link_libraries(${T} PUBLIC CLI11)
    add_gtest(${T})

    if(CLI11_SINGLE_FILE AND CLI11_SINGLE_FILE_TESTS)
        add_executable(${T}_Single ${T}.cpp)
        target_link_libraries(${T}_Single PUBLIC CLI11_SINGLE)
        add_gtest(${T}_Single)
        set_target_properties(${T}_Single
            PROPERTIES
            FOLDER "Tests Single File")
    endif()

endforeach()

foreach(T ${CLI11_MULTIONLY_TESTS})

    add_executable(${T} ${T}.cpp ${CLI11_headers})
    target_link_libraries(${T} PUBLIC CLI11)
    add_gtest(${T})

endforeach()


# Link test (build error if inlines missing)
add_library(link_test_1 link_test_1.cpp)
target_link_libraries(link_test_1 PUBLIC CLI11)
set_target_properties(link_test_1 PROPERTIES FOLDER "Tests")
add_executable(link_test_2 link_test_2.cpp)
target_link_libraries(link_test_2 PUBLIC CLI11 link_test_1)
add_gtest(link_test_2)

# Add informational printout
# Force this to be in a standard location so CTest can find it
add_executable(informational informational.cpp)
target_link_libraries(informational PUBLIC CLI11)
set_target_properties(informational PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}"
    )

# Adding this printout to CTest
file(WRITE "${PROJECT_BINARY_DIR}/CTestCustom.cmake"
    "set(CTEST_CUSTOM_PRE_TEST \"${CMAKE_BINARY_DIR}/informational\")"
    )

# Add boost to test boost::optional if available
find_package(Boost 1.35)
if(Boost_FOUND)
    target_link_libraries(informational PUBLIC Boost::boost)
    target_link_libraries(OptionalTest PUBLIC Boost::boost)

    # Enforce Boost::Optional even if __has_include is missing on your compiler
    target_compile_definitions(informational PUBLIC CLI11_BOOST_OPTIONAL)
    target_compile_definitions(OptionalTest PUBLIC CLI11_BOOST_OPTIONAL)
endif()

