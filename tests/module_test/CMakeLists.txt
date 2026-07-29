# Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
# under NSF AWARD 1414736 and by the respective contributors.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28...4.2)
set(CMAKE_CXX_EXTENSIONS OFF)

project(CLI11-module-test)

include(CTest)

if(CLI11_DIR)
  set(CMAKE_PREFIX_PATH ${CLI11_DIR})
endif()

# Default only; the outer build passes its own standard so the module BMI matches
set(CMAKE_CXX_STANDARD
    23
    CACHE STRING "The C++ standard to use")

# Test the CLI11 CMake package config
find_package(CLI11 2.5 REQUIRED)

# Test inclusion of CLI11 in modules
add_executable(module-include-test module_include_test.cpp)

target_sources(module-include-test PUBLIC FILE_SET cmodule TYPE CXX_MODULES FILES cmodule.ixx)

target_link_libraries(module-include-test CLI11::CLI11)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(module-include-test PUBLIC -fmodules-ts)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if(CLI11_FORCE_LIBCXX)
    set_property(
      TARGET module-include-test
      APPEND_STRING
      PROPERTY LINK_FLAGS -stdlib=libc++)
    target_link_libraries(module-include-test c++abi)
    #target_link_libraries(module-include-test c++)
  endif()
endif()

add_test(NAME module-include-test1 COMMAND module-include-test one)
set_property(TEST module-include-test1 PROPERTY PASS_REGULAR_EXPRESSION "OK: export module")

# Test the CLI11 Module target if available
if(CLI11_MODULES)
  # Test the target
  add_executable(module-test module_test.cpp)

  target_link_libraries(module-test CLI11::CLI11_Module)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(module-test PUBLIC -fmodules-ts)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CLI11_FORCE_LIBCXX)
      set_property(
        TARGET module-test
        APPEND_STRING
        PROPERTY LINK_FLAGS -stdlib=libc++)
      target_link_libraries(module-test c++abi)
      # target_link_libraries(module-test c++)
    endif()
  endif()

  add_test(NAME module-test1 COMMAND module-test one)
  set_property(TEST module-test1 PROPERTY PASS_REGULAR_EXPRESSION
                                          "OK: import cli11 module\nvalue = one")

  # Long options through a subcommand (see #1318)
  add_test(NAME module-test-long-opts COMMAND module-test one configure --script-path foo
                                              --build-dir bar)
  set_property(
    TEST module-test-long-opts
    PROPERTY PASS_REGULAR_EXPRESSION "subcommand = configure\nscript_path = foo\nbuild_dir = bar")

  # Short options through a subcommand
  add_test(NAME module-test-short-opts COMMAND module-test one install -B bld -I prefix)
  set_property(
    TEST module-test-short-opts
    PROPERTY PASS_REGULAR_EXPRESSION "subcommand = install\nbuild_dir = bld\ninstall_dir = prefix")

  # Long option in an option group
  add_test(NAME module-test-option-group COMMAND module-test one --output json)
  set_property(TEST module-test-option-group PROPERTY PASS_REGULAR_EXPRESSION "output = json")
endif()
