if(NOT EXISTS "${CLI11_SOURCE_DIR}/extern/googletest/CMakeLists.txt")
    message(FATAL_ERROR "You have requested tests be built, but googletest is not downloaded. Please run:
    git submodule update --init")
endif()

list(APPEND CMAKE_MODULE_PATH "${CLI11_SOURCE_DIR}/cmake")

# If submodule is available, add sanitizers
# Set SANITIZE_ADDRESS, SANITIZE_MEMORY, SANITIZE_THREAD or SANITIZE_UNDEFINED
if(EXISTS "${CLI11_SOURCE_DIR}/extern/sanitizers/cmake/FindSanitizers.cmake")
    list(APPEND CMAKE_MODULE_PATH "${CLI11_SOURCE_DIR}/extern/sanitizers/cmake")
    find_package(Sanitizers)
    if(SANITIZE_ADDRESS)
        message(STATUS "You might want to use \"${ASan_WRAPPER}\" to run your program")
    endif()
else()
    macro(add_sanitizers)
    endmacro()
endif()

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
    FormatterTest
    NewParseTest
    OptionalTest
    DeprecatedTest
    )

if(WIN32)
    list(APPEND CLI11_TESTS WindowsTest)
endif()

set(CLI11_MULTIONLY_TESTS
    TimerTest
    )

# Only affects current directory, so safe
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(T ${CLI11_TESTS})

    add_executable(${T} ${T}.cpp ${CLI11_headers})
    add_sanitizers(${T})
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
    add_sanitizers(${T})
    target_link_libraries(${T} PUBLIC CLI11)
    add_gtest(${T})

endforeach()

# Add -Wno-deprecated-declarations to DeprecatedTest
if(NOT MSVC)
    target_compile_options(DeprecatedTest PRIVATE -Wno-deprecated-declarations)
    if(TARGET DeprecatedTest_Single)
        target_compile_options(DeprecatedTest_Single PRIVATE -Wno-deprecated-declarations)
    endif()
endif()

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
find_package(Boost 1.58)
if(Boost_FOUND)
    target_link_libraries(informational PUBLIC Boost::boost)
    target_link_libraries(OptionalTest PUBLIC Boost::boost)

    # Enforce Boost::Optional even if __has_include is missing on your compiler
    target_compile_definitions(informational PUBLIC CLI11_BOOST_OPTIONAL)
    target_compile_definitions(OptionalTest PUBLIC CLI11_BOOST_OPTIONAL)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    include(CodeCoverage)
    setup_target_for_coverage(
        NAME CLI11_coverage
        EXECUTABLE ctest
        DEPENDENCIES
          ${CLI11_TESTS}
          ${CLI11_MULTIONLY_TESTS})
endif()

