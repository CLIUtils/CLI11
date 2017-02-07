#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>


// C standard library
// Only needed for existence checking
// Could be swapped for filesystem in C++17
#include <sys/types.h>
#include <sys/stat.h>

namespace CLI {


/// Check for an existing file
bool ExistingFile(std::string filename) {
//    std::fstream f(name.c_str());
//    return f.good();
//    Fastest way according to http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
    struct stat buffer;   
    return (stat(filename.c_str(), &buffer) == 0); 
}

/// Check for an existing directory
bool ExistingDirectory(std::string filename) {
    struct stat buffer;   
    if(stat(filename.c_str(), &buffer) == 0 && (buffer.st_mode & S_IFDIR) )
        return true;
    return false;
}

/// Check for a non-existing path
bool NonexistentPath(std::string filename) {
    struct stat buffer;
    return stat(filename.c_str(), &buffer) != 0;
}


}
