#pragma once

// Distributed under the LGPL v2.1 license.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <string>
#include <iostream>
#include <functional>
#include "CLI/TypeTools.hpp"

// C standard library
// Only needed for existence checking
// Could be swapped for filesystem in C++17
#include <sys/types.h>
#include <sys/stat.h>

namespace CLI {


/// @defgroup validator_group Validators
/// @brief Some validators that are provided
///
/// These are simple `bool(std::string)` validators that are useful.
/// @{

/// Check for an existing file
bool ExistingFile(std::string filename) {
    struct stat buffer;   
    bool exist = stat(filename.c_str(), &buffer) == 0; 
    bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
    if(!exist) {
        std::cerr << "File does not exist: " << filename << std::endl;
        return false;
    } else if (is_dir) {
        std::cerr << "File is actually a directory: " << filename << std::endl;
        return false;
    } else {
        return true;
    }
}

/// Check for an existing directory
bool ExistingDirectory(std::string filename) {
    struct stat buffer;   
    bool exist = stat(filename.c_str(), &buffer) == 0; 
    bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
    if(!exist) {
        std::cerr << "Directory does not exist: " << filename << std::endl;
        return false;
    } else if (is_dir) {
        return true;
    } else {
        std::cerr << "Directory is actually a file: " << filename << std::endl;
        return false;
    }
}


/// Check for a non-existing path
bool NonexistentPath(std::string filename) {
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
template<typename T>
std::function<bool(std::string)> Range(T min, T max) {
    return [min, max](std::string input){
        T val;
        detail::lexical_cast(input, val);
        return val >= min && val <= max;
    };
}

/// Range of one value is 0 to value
template<typename T>
std::function<bool(std::string)> Range(T max) {
    return Range((T) 0, max);
}

/// @}

}
