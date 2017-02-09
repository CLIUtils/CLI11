#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <iostream>


// C standard library
// Only needed for existence checking
// Could be swapped for filesystem in C++17
#include <sys/types.h>
#include <sys/stat.h>

namespace CLI {


/// Check for an existing file
bool ExistingFile(std::string filename) {
    struct stat buffer;   
    bool exist = stat(filename.c_str(), &buffer) == 0; 
    bool is_dir = buffer.st_mode & S_IFDIR;
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
    bool is_dir = buffer.st_mode & S_IFDIR;
    if(!exist) {
        std::cerr << "Directory does not exist: " << filename << std::endl;
        return false;
    } else if (is_dir) {
        return true;
    } else {
        std::cerr << "Directory is actually a file: " << filename << std::endl;
        return true;
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


}
