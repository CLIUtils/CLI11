list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if (CMAKE_VERSION VERSION_GREATER 3.10)
  include(FetchContent)
  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_SHALLOW 1
    GIT_TAG 2fe3bd9)
  FetchContent_Declare(sanitizers
    GIT_REPOSITORY https://github.com/arsenm/sanitizers-cmake.git
    GIT_SHALLOW 1
    GIT_TAG 99e159e)
  FetchContent_GetProperties(googletest)
  FetchContent_GetProperties(sanitizers)
  if (NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
  endif()
  if (NOT sanitizers_POPULATED)
    FetchContent_Populate(sanitizers)
  endif()
else()
  set(googletest_SOURCE_DIR "${PROJECT_SOURCE_DIR}/extern/googletest")
  set(googletest_BINARY_DIR "${PROJECT_BINARY_DIR}/extern/googletest")
  set(sanitizers_SOURCE_DIR "${PROJECT_SOURCE_DIR}/extern/sanitizers")
  set(sanitizers_BINARY_DIR "${PROJECT_BINARY_DIR}/extern/sanitizers")
endif()

if(NOT EXISTS "${googletest_SOURCE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "You have requested tests be built,"
    "but googletest is not downloaded. Please run:\n"
    "\tgit submodule update --init")
endif()

# If submodule is available, add sanitizers
# Set SANITIZE_ADDRESS, SANITIZE_MEMORY, SANITIZE_THREAD or SANITIZE_UNDEFINED
if(IS_DIRECTORY "${sanitizers_SOURCE_DIR}/cmake")
  list(APPEND CMAKE_MODULE_PATH "${sanitizers_SOURCE_DIR}/cmake")
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
  TrueFalseTest
  OptionGroupTest)

if(WIN32)
    list(APPEND CLI11_TESTS WindowsTest)
endif()

set(CLI11_MULTIONLY_TESTS TimerTest)

# Only affects current directory, so safe
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

foreach(T IN LISTS CLI11_TESTS)
  add_executable(${T} ${T}.cpp ${CLI11_headers})
  add_sanitizers(${T})
  target_link_libraries(${T} PUBLIC CLI11 CLI11_warnings)
  add_gtest(${T})

  if(CLI11_SINGLE_FILE AND CLI11_SINGLE_FILE_TESTS)
    add_executable(${T}_Single ${T}.cpp)
    target_link_libraries(${T}_Single PUBLIC CLI11_SINGLE)
    add_gtest(${T}_Single)
    set_property(TARGET ${T}_Single PROPERTY FOLDER "Tests Single File")
  endif()
endforeach()

foreach(T IN LISTS CLI11_MULTIONLY_TESTS)
  add_executable(${T} ${T}.cpp ${CLI11_headers})
  add_sanitizers(${T})
  target_link_libraries(${T} PUBLIC CLI11)
  add_gtest(${T})
endforeach()

# Add -Wno-deprecated-declarations to DeprecatedTest
set(no-deprecated-declarations $<IF:$<CXX_COMPILER_ID:MSVC>,/wd4996,-Wno-deprecated-declarations>)
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
add_gtest(link_test_2)
if (CLI11_FORCE_LIBCXX)
  set_property(TARGET link_test_1 APPEND_STRING
    PROPERTY LINK_FLAGS -stdlib=libc++)
  set_property(TARGET link_test_2 APPEND_STRING
    PROPERTY LINK_FLAGS -stdlib=libc++)
endif()


# Add informational printout
add_executable(informational informational.cpp)
target_link_libraries(informational PUBLIC CLI11)
if (CLI11_FORCE_LIBCXX)
  set_property(TARGET informational APPEND_STRING
    PROPERTY LINK_FLAGS -stdlib=libc++)
endif()

# Adding this printout to CTest
file(GENERATE OUTPUT "${PROJECT_BINARY_DIR}/CTestCustom.cmake"
  CONTENT [[
  set(CTEST_CUSTOM_PRE_TEST "$<TARGET_FILE:informational>")
]])

# Add boost to test boost::optional if available
find_package(Boost 1.61)
set(boost-optional $<$<BOOL:${Boost_FOUND}>:CLI11_BOOST_OPTIONAL>)
set(boost-include $<$<BOOL:${Boost_FOUND}>:${Boost_INCLUDE_DIRS}>)
target_include_directories(informational PRIVATE ${boost-include})
target_compile_definitions(informational PRIVATE ${boost-optional})

target_include_directories(OptionalTest PRIVATE ${boost-include})
target_compile_definitions(OptionalTest PRIVATE ${boost-optional})
if(TARGET Boost::boost)
    target_link_libraries(informational PRIVATE Boost::boost)
    target_link_libraries(OptionalTest PRIVATE Boost::boost)
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

