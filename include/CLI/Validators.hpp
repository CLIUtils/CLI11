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
/// These are simple `void(std::string&)` validators that are useful. They throw
/// a ValidationError if they fail (or the normally expected error if the cast fails)
/// @{

/// Check for an existing file
inline std::string ExistingFile(const std::string &filename) {
    struct stat buffer;
    bool exist = stat(filename.c_str(), &buffer) == 0;
    bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
    if(!exist) {
        return "File does not exist: " + filename;
    } else if(is_dir) {
        return "File is actually a directory: " + filename;
    }
    return std::string();
}

/// Check for an existing directory
inline std::string ExistingDirectory(const std::string &filename) {
    struct stat buffer;
    bool exist = stat(filename.c_str(), &buffer) == 0;
    bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
    if(!exist) {
        return "Directory does not exist: " + filename;
    } else if(!is_dir) {
        return "Directory is actually a file: " + filename;
    }
    return std::string();
}

/// Check for an existing path
inline std::string ExistingPath(const std::string &filename) {
    struct stat buffer;
    bool const exist = stat(filename.c_str(), &buffer) == 0;
    if(!exist) {
        return "Path does not exist: " + filename;
    }
    return std::string();
}

/// Check for a non-existing path
inline std::string NonexistentPath(const std::string &filename) {
    struct stat buffer;
    bool exist = stat(filename.c_str(), &buffer) == 0;
    if(exist) {
        return "Path already exists: " + filename;
    }
    return std::string();
}

/// Produce a range validator function
template <typename T> std::function<std::string(const std::string &)> Range(T min, T max) {
    return [min, max](std::string input) {
        T val;
        detail::lexical_cast(input, val);
        if(val < min || val > max)
            return "Value " + input + " not in range " + std::to_string(min) + " to " + std::to_string(max);

        return std::string();
    };
}

/// Range of one value is 0 to value
template <typename T> std::function<std::string(const std::string &)> Range(T max) {
    return Range(static_cast<T>(0), max);
}

/// @}

} // namespace CLI
