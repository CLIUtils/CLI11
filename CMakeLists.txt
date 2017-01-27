cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(CLI11 CXX)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# C++11 without GNU extensions
# Requires CMAKE 3.1+ for Mac
if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 3.1)
    add_compile_options(-std=c++11)
else()
    cmake_policy(VERSION 3.1) # Needed for Mac
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
# be moderately paranoid with flags
add_compile_options(-pedantic -Wall -Wextra)

include_directories(include)
set(headers "${PROJECT_SOURCE_DIR}/include/CLI.hpp")
enable_testing()

add_subdirectory(tests)
add_subdirectory(examples)
