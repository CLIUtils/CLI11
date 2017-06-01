#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include "CLI/TypeTools.hpp"
#include <functional>
#include <iostream>
#include <string>

// C standard library
// Only needed for existence checking
// Could be swapped for filesystem in C++17
#include <sys/stat.h>
#include <sys/types.h>

namespace CLI {

/// @defgroup validator_group Validators
/// @brief Some validators that are provided
///
/// These are simple `bool(std::string)` validators that are useful.
/// @{

/// Check for an existing file
inline bool ExistingFile(std::string filename) {
    struct stat buffer;
    bool exist = stat(filename.c_str(), &buffer) == 0;
    bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
    if(!exist) {
        std::cerr << "File does not exist: " << filename << std::endl;
        return false;
    } else if(is_dir) {
        std::cerr << "File is actually a directory: " << filename << std::endl;
        return false;
    } else {
        return true;
    }
}

/// Check for an existing directory
inline bool ExistingDirectory(std::string filename) {
    struct stat buffer;
    bool exist = stat(filename.c_str(), &buffer) == 0;
    bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
    if(!exist) {
        std::cerr << "Directory does not exist: " << filename << std::endl;
        return false;
    } else if(is_dir) {
        return true;
    } else {
        std::cerr << "Directory is actually a file: " << filename << std::endl;
        return false;
    }
}

/// Check for a non-existing path
inline bool NonexistentPath(std::string filename) {
    struct stat buffer;
    bool exist = stat(filename.c_str(), &buffer) == 0;
    if(!exist) {
        return true;
    } else {
        std::cerr << "Path exists: " << filename << std::endl;
        return false;
    }
}

/// Produce a range validator function
template <typename T> std::function<bool(std::string)> Range(T min, T max) {
    return [min, max](std::string input) {
        T val;
        detail::lexical_cast(input, val);
        return val >= min && val <= max;
    };
}

/// Range of one value is 0 to value
template <typename T> std::function<bool(std::string)> Range(T max) { return Range(static_cast<T>(0), max); }

/// @}

} // namespace CLI
