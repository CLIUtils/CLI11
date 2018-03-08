project(PackageTest CXX)
cmake_minimum_required(VERSION 3.1)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "${CMAKE_PREFIX_PATH}")

find_package(CLI11 CONFIG REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example CLI11::CLI11)
