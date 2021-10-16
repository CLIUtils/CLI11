cmake_minimum_required(VERSION 3.4)
# Note: this is a header only library. If you have an older CMake than 3.4,
# just add the CLI11/include directory and that's all you need to do.

# Make sure users don't get warnings on a tested (3.4 to 3.22) version
# of CMake. For most of the policies, the new version is better (hence the change).
# We don't use the 3.4...3.21 syntax because of a bug in an older MSVC's
# built-in and modified CMake 3.11
if(${CMAKE_VERSION} VERSION_LESS 3.22)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
  cmake_policy(VERSION 3.22)
endif()

set(VERSION_REGEX "#define CLI11_VERSION[ \t]+\"(.+)\"")

# Read in the line containing the version
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/include/CLI/Version.hpp" VERSION_STRING
     REGEX ${VERSION_REGEX})

# Pick out just the version
string(REGEX REPLACE ${VERSION_REGEX} "\\1" VERSION_STRING "${VERSION_STRING}")

# Add the project
project(
  CLI11
  LANGUAGES CXX
  VERSION ${VERSION_STRING})

# Print the version number of CMake if this is the main project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  message(STATUS "CMake ${CMAKE_VERSION}")

  find_package(Doxygen)

  if(CMAKE_VERSION VERSION_LESS 3.10)
    message(STATUS "CMake 3.10+ adds Doxygen support. Update CMake to build documentation")
  elseif(NOT Doxygen_FOUND)
    message(STATUS "Doxygen not found, building docs has been disabled")
  endif()

  include(CTest)
else()
  if(NOT DEFINED BUILD_TESTING)
    set(BUILD_TESTING OFF)
  endif()
endif()

include(CMakeDependentOption)
include(GNUInstallDirs)

if(NOT CMAKE_VERSION VERSION_LESS 3.11)
  include(FetchContent)
endif()

list(APPEND force-libcxx "CMAKE_CXX_COMPILER_ID STREQUAL \"Clang\"")
list(APPEND force-libcxx "CMAKE_SYSTEM_NAME STREQUAL \"Linux\"")
list(APPEND force-libcxx "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME")

list(APPEND build-docs "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME")
list(APPEND build-docs "NOT CMAKE_VERSION VERSION_LESS 3.11")
list(APPEND build-docs "Doxygen_FOUND")

# Necessary to support paths with spaces, see #457
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/docs")
  set(docs_EXIST TRUE)
else()
  set(docs_EXIST FALSE)
endif()
list(APPEND build-docs "docs_EXIST")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
  set(examples_EXIST TRUE)
else()
  set(examples_EXIST FALSE)
endif()

option(CLI11_WARNINGS_AS_ERRORS "Turn all warnings into errors (for CI)")
option(CLI11_SINGLE_FILE "Generate a single header file")
cmake_dependent_option(CLI11_SANITIZERS "Download the sanitizers CMake config" OFF
                       "NOT CMAKE_VERSION VERSION_LESS 3.11" OFF)

cmake_dependent_option(CLI11_BUILD_DOCS "Build CLI11 documentation" ON "${build-docs}" OFF)

cmake_dependent_option(CLI11_BUILD_TESTS "Build CLI11 tests" ON
                       "BUILD_TESTING;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF)

cmake_dependent_option(CLI11_BUILD_EXAMPLES "Build CLI11 examples" ON
                       "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME;${examples_EXIST}" OFF)

cmake_dependent_option(CLI11_BUILD_EXAMPLES_JSON "Build CLI11 json example" OFF
                       "CLI11_BUILD_EXAMPLES;NOT CMAKE_VERSION VERSION_LESS 3.11" OFF)

cmake_dependent_option(CLI11_SINGLE_FILE_TESTS "Duplicate all the tests for a single file build"
                       OFF "BUILD_TESTING;CLI11_SINGLE_FILE" OFF)

cmake_dependent_option(CLI11_INSTALL "Install the CLI11 folder to include during install process"
                       ON "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF)

cmake_dependent_option(
  CLI11_FORCE_LIBCXX "Force clang to use libc++ instead of libstdc++ (Linux only)" OFF
  "${force-libcxx}" OFF)

cmake_dependent_option(
  CLI11_CUDA_TESTS "Build the tests with NVCC to check for warnings there - requires CMake 3.9+"
  OFF "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF)

cmake_dependent_option(
  CLI11_CLANG_TIDY "Look for and use Clang-Tidy" OFF
  "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME;NOT CMAKE_VERSION VERSION_LESS 3.6" OFF)
set(CLI11_CLANG_TIDY_OPTIONS
    ""
    CACHE STRING "Clang tidy options, such as -fix, semicolon separated")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
endif()

if(NOT DEFINED CMAKE_CXX_EXTENSIONS)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD_REQUIRED)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

# Allow IDE's to group targets into folders
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Special target that adds warnings. Is not exported.
add_library(CLI11_warnings INTERFACE)

set(unix-warnings -Wall -Wextra -pedantic -Wshadow -Wsign-conversion -Wswitch-enum)

# Buggy in GCC 4.8
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
  list(APPEND unix-warnings -Weffc++)
endif()

target_compile_options(
  CLI11_warnings
  INTERFACE $<$<BOOL:${CLI11_FORCE_LIBCXX}>:-stdlib=libc++>
            $<$<CXX_COMPILER_ID:MSVC>:/W4
            $<$<BOOL:${CLI11_WARNINGS_AS_ERRORS}>:/WX>>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:${unix-warnings}
            $<$<BOOL:${CLI11_WARNINGS_AS_ERRORS}>:-Werror>>)

if(NOT CMAKE_VERSION VERSION_LESS 3.13)
  target_link_options(CLI11_warnings INTERFACE $<$<BOOL:${CLI11_FORCE_LIBCXX}>:-stdlib=libc++>)
endif()

# Allow IDE's to group targets into folders
add_library(CLI11 INTERFACE)
add_library(CLI11::CLI11 ALIAS CLI11) # for add_subdirectory calls

# Duplicated because CMake adds the current source dir if you don't.
target_include_directories(CLI11 INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                           $<INSTALL_INTERFACE:include>)

if(CMAKE_VERSION VERSION_LESS 3.8)
  # This might not be a complete list
  target_compile_features(
    CLI11
    INTERFACE cxx_lambdas
              cxx_nullptr
              cxx_override
              cxx_range_for
              cxx_right_angle_brackets
              cxx_strong_enums
              cxx_constexpr
              cxx_auto_type)
else()
  target_compile_features(CLI11 INTERFACE cxx_std_11)
endif()

# To see in IDE, headers must be listed for target
set(header-patterns "${PROJECT_SOURCE_DIR}/include/CLI/*")
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND NOT CMAKE_VERSION VERSION_LESS 3.12)
  list(INSERT header-patterns 0 CONFIGURE_DEPENDS)
endif()

file(GLOB CLI11_headers ${header-patterns})

# Allow tests to be run on CUDA
if(CLI11_CUDA_TESTS)
  enable_language(CUDA)

  # Print out warning and error numbers
  set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcudafe --display_error_number")
endif()

# Prepare Clang-Tidy
if(CLI11_CLANG_TIDY)
  find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable" REQUIRED)

  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" ${CLI11_CLANG_TIDY_OPTIONS})
endif()

# This folder should be installed
if(CLI11_INSTALL)
  install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

  # Make an export target
  install(TARGETS CLI11 EXPORT CLI11Targets)

  # Use find_package on the installed package
  # Since we have no custom code, we can directly write this
  # to Config.cmake (otherwise we'd have a custom config and would
  # import Targets.cmake

  # Add the version in a CMake readable way
  configure_file("cmake/CLI11ConfigVersion.cmake.in" "CLI11ConfigVersion.cmake" @ONLY)

  # Make version available in the install
  install(FILES "${PROJECT_BINARY_DIR}/CLI11ConfigVersion.cmake"
          DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/CLI11")

  # Install the export target as a file
  install(
    EXPORT CLI11Targets
    FILE CLI11Config.cmake
    NAMESPACE CLI11::
    DESTINATION "${CMAKE_INSTALL_DATADIR}/cmake/CLI11")

  # Use find_package on the installed package
  export(
    TARGETS CLI11
    NAMESPACE CLI11::
    FILE CLI11Targets.cmake)

  include(cmake/CLI11GeneratePkgConfig.cmake)

  # Register in the user cmake package registry
  export(PACKAGE CLI11)
endif()

if(CLI11_SINGLE_FILE)
  # Single file test
  if(CMAKE_VERSION VERSION_LESS 3.12)
    find_package(PythonInterp REQUIRED)
    add_executable(Python::Interpreter IMPORTED)
    set_target_properties(Python::Interpreter PROPERTIES IMPORTED_LOCATION "${PYTHON_EXECUTABLE}"
                                                         VERSION "${PYTHON_VERSION_STRING}")
  else()
    find_package(
      Python
      COMPONENTS Interpreter
      REQUIRED)
  endif()

  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include")
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp"
    COMMAND
      Python::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/scripts/MakeSingleHeader.py"
      ${CLI11_headers} --main "${CMAKE_CURRENT_SOURCE_DIR}/CLI11.hpp.in" --output
      "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp" --version "${CLI11_VERSION}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/CLI/CLI.hpp" ${CLI11_headers})
  add_custom_target(CLI11-generate-single-file ALL
                    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp")
  set_property(TARGET CLI11-generate-single-file PROPERTY FOLDER "Scripts")
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/CLI11.hpp" DESTINATION include)
  add_library(CLI11_SINGLE INTERFACE)
  target_link_libraries(CLI11_SINGLE INTERFACE CLI11)
  add_dependencies(CLI11_SINGLE CLI11-generate-single-file)
  target_compile_definitions(CLI11_SINGLE INTERFACE -DCLI11_SINGLE_FILE)
  target_include_directories(
    CLI11_SINGLE INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/>
                           $<INSTALL_INTERFACE:include>)
endif()

if(CLI11_BUILD_TESTS)
  include(CTest)
  add_subdirectory(tests)
endif()

if(CLI11_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(CLI11_BUILD_DOCS)
  add_subdirectory(docs)
endif()

# From a build system, this might not be included.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/book")
  add_subdirectory(book)
endif()

# Packaging support
set(CPACK_PACKAGE_VENDOR "github.com/CLIUtils/CLI11")
set(CPACK_PACKAGE_CONTACT "https://${CPACK_PACKAGE_VENDOR}")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR}) # Automatic in CMake 3.12+
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR}) # Automatic in CMake 3.12+
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH}) # Automatic in CMake 3.12+
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Command line parser with simple and intuitive interface")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CLI11.CPack.Description.txt")
set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")

# CPack collects *everything* except what's listed here.
set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /dist
    /.*build.*
    /\\\\.DS_Store
    /.*\\\\.egg-info
    /var
    /azure-pipelines.yml
    /.ci
    /docs
    /examples
    /test_package
    /book
    /.travis.yml
    .swp
    /.all-contributorsrc
    /.appveyor.yml
    /.pre-commit.*yaml)

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
set(CPACK_DEBIAN_PACKAGE_NAME "libcli11-dev")

include(CPack)
