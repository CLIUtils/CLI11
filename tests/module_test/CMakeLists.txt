# Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
# under NSF AWARD 1414736 and by the respective contributors.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.14...4.2)

project(CLI11-module-test)

include(CTest)

if(CLI11_DIR)
  set(CMAKE_PREFIX_PATH ${CLI11_DIR})
endif()

set(CMAKE_CXX_STANDARD 23)
# Test the CLI11 CMake package config
find_package(CLI11 2.5 REQUIRED)

# Test the target
add_executable(module-test module_test.cpp)

target_sources(module-test PUBLIC FILE_SET cmodule TYPE CXX_MODULES FILES cmodule.ixx)

target_link_libraries(module-test CLI11::CLI11)
target_compile_options(module-test PUBLIC -fmodules-ts)

add_test(NAME module-test1 COMMAND module-test one)
set_property(TEST module-test1 PROPERTY PASS_REGULAR_EXPRESSION "OK: export module")
