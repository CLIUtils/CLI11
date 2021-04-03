list(APPEND CMAKE_MODULE_PATH "${CLI11_SOURCE_DIR}/cmake")

if(CLI11_SANITIZERS)
    message(STATUS "Using arsenm/sanitizers-cmake")
    FetchContent_Declare(sanitizers
        GIT_REPOSITORY https://github.com/arsenm/sanitizers-cmake.git
        GIT_SHALLOW 1
        GIT_TAG 99e159e)

    FetchContent_GetProperties(sanitizers)

    if (NOT sanitizers_POPULATED)
        FetchContent_Populate(sanitizers)
    endif()

    list(APPEND CMAKE_MODULE_PATH "${sanitizers_SOURCE_DIR}/cmake")

    find_package(Sanitizers)
    if(SANITIZE_ADDRESS)
        message(STATUS "You might want to use \"${ASan_WRAPPER}\" to run your program")
    endif()
else()
    macro(add_sanitizers)
    endmacro()
endif()


# Add boost to test boost::optional (currently explicitly requested)"
option(CLI11_BOOST "Turn on boost test (currently may fail with Boost 1.70)" OFF)
if(CLI11_BOOST)
    find_package(Boost 1.61 REQUIRED)
endif()
set(boost-optional-def $<$<BOOL:${Boost_FOUND}>:CLI11_BOOST_OPTIONAL>)

set(CLI11_TESTS
    HelpersTest
    ConfigFileTest
    OptionTypeTest
    SimpleTest
    AppTest
    SetTest
    TransformTest
    CreationTest
    SubcommandTest
    HelpTest
    FormatterTest
    NewParseTest
    OptionalTest
    DeprecatedTest
    StringParseTest
    ComplexTypeTest
    TrueFalseTest
    OptionGroupTest
    )

if(WIN32)
    list(APPEND CLI11_TESTS WindowsTest)
endif()

if (Boost_FOUND)
   list(APPEND CLI11_TESTS BoostOptionTypeTest)
endif()

set(CLI11_MULTIONLY_TESTS TimerTest)

add_library(catch_main main.cpp)
target_include_directories(catch_main PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Currently a required download; could be make to look for existing Catch2, but
# that would require changing the includes. FetchContent would be better, but
# requires newer CMake.

set(url https://github.com/philsquared/Catch/releases/download/v2.13.4/catch.hpp)
file(DOWNLOAD ${url} "${CMAKE_CURRENT_BINARY_DIR}/catch.hpp" STATUS status EXPECTED_HASH SHA256=6e0fa3dd160891a01c1f3b34e8bcd6e0140abe08eca022e390027f27dec2050b)
list(GET status 0 error)
if(error)
    message(FATAL_ERROR "Could not download ${url}")
endif()
target_include_directories(catch_main PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")

# Target must already exist
macro(add_catch_test TESTNAME)
    target_link_libraries(${TESTNAME} PUBLIC catch_main)

    add_test(${TESTNAME} ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER "Tests")
    if (CLI11_FORCE_LIBCXX)
       set_property(TARGET ${T} APPEND_STRING
         PROPERTY LINK_FLAGS -stdlib=libc++)
     endif()
endmacro()

foreach(T IN LISTS CLI11_TESTS)
    if(CLI11_CUDA_TESTS)
        set_property(
            SOURCE ${T}.cpp
            PROPERTY LANGUAGE CUDA
            )
    endif()
    add_executable(${T} ${T}.cpp ${CLI11_headers})
    add_sanitizers(${T})
    if(NOT CLI11_CUDA_TESTS)
        target_link_libraries(${T} PRIVATE CLI11_warnings)
    endif()
    target_link_libraries(${T} PRIVATE CLI11)
    add_catch_test(${T})

    if(CLI11_SINGLE_FILE AND CLI11_SINGLE_FILE_TESTS)
        add_executable(${T}_Single ${T}.cpp)
        target_link_libraries(${T}_Single PRIVATE CLI11_SINGLE)
        add_catch_test(${T}_Single)
        set_property(TARGET ${T}_Single PROPERTY FOLDER "Tests Single File")
    endif()
endforeach()

foreach(T IN LISTS CLI11_MULTIONLY_TESTS)
    add_executable(${T} ${T}.cpp ${CLI11_headers})
    add_sanitizers(${T})
    target_link_libraries(${T} PUBLIC CLI11)
    add_catch_test(${T})
endforeach()

# Add -Wno-deprecated-declarations to DeprecatedTest
set(no-deprecated-declarations
    $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wno-deprecated-declarations>
    )
target_compile_options(DeprecatedTest
    PRIVATE
    ${no-deprecated-declarations})
if (TARGET DeprecatedTest_Single)
    target_compile_options(DeprecatedTest_Single
        PRIVATE
        ${no-deprecated-declarations})
endif()

# Link test (build error if inlines missing)
add_library(link_test_1 link_test_1.cpp)
target_link_libraries(link_test_1 PUBLIC CLI11)
set_target_properties(link_test_1 PROPERTIES FOLDER "Tests")
add_executable(link_test_2 link_test_2.cpp)
target_link_libraries(link_test_2 PUBLIC CLI11 link_test_1)
add_catch_test(link_test_2)
if(CLI11_FORCE_LIBCXX)
    set_property(TARGET link_test_1 APPEND_STRING
        PROPERTY LINK_FLAGS -stdlib=libc++)
    set_property(TARGET link_test_2 APPEND_STRING
        PROPERTY LINK_FLAGS -stdlib=libc++)
endif()

# Add informational printout
add_executable(informational informational.cpp)
target_link_libraries(informational PUBLIC CLI11)
if(CLI11_FORCE_LIBCXX)
    set_property(TARGET informational APPEND_STRING
        PROPERTY LINK_FLAGS -stdlib=libc++)
endif()

# Force this to be in a standard location so CTest can find it
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

target_compile_definitions(informational PRIVATE ${boost-optional-def})
target_compile_definitions(OptionalTest PRIVATE ${boost-optional-def})

message(STATUS "Boost libs=${Boost_INCLUDE_DIRS}")

if(TARGET Boost::boost)
    message(STATUS "including boost target")
    target_link_libraries(informational PRIVATE Boost::boost)
    target_link_libraries(OptionalTest PRIVATE Boost::boost)
    target_link_libraries(BoostOptionTypeTest PRIVATE Boost::boost)
    if(CLI11_SINGLE_FILE AND CLI11_SINGLE_FILE_TESTS)
       target_link_libraries(OptionalTest_Single PRIVATE Boost::boost)
       target_link_libraries(BoostOptionTypeTest_Single PRIVATE Boost::boost)
    endif()
elseif(BOOST_FOUND)
message(STATUS "no boost target")
    target_include_directories(informational PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(OptionalTest PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(BoostOptionTypeTest PRIVATE ${Boost_INCLUDE_DIRS})
    if(CLI11_SINGLE_FILE AND CLI11_SINGLE_FILE_TESTS)
        target_include_directories(OptionalTest_Single PRIVATE ${Boost_INCLUDE_DIRS})
        target_include_directories(BoostOptionTypeTest_Single PRIVATE ${Boost_INCLUDE_DIRS})
     endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL Coverage)
    include(CodeCoverage)
    setup_target_for_coverage(
        NAME CLI11_coverage
        EXECUTABLE ctest
        DEPENDENCIES
          ${CLI11_TESTS}
          ${CLI11_MULTIONLY_TESTS}
        )
endif()

