#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <functional>
#include <vector>


// C standard library
// Only needed for existence checking
// Could be swapped for filesystem in C++17
#include <sys/types.h>
#include <sys/stat.h>

namespace CLI {

namespace detail {

struct Combiner {
    int num;
    bool required;
    bool defaulted;
    std::vector<std::function<bool(std::string)>> validators;

    /// Can be or-ed together
    Combiner operator | (Combiner b) const {
        Combiner self;
        self.num = std::min(num, b.num) == -1 ? -1 : std::max(num, b.num);
        self.required = required || b.required;
        self.defaulted = defaulted || b.defaulted;
        self.validators.reserve(validators.size() + b.validators.size());
        self.validators.insert(self.validators.end(), validators.begin(), validators.end());
        self.validators.insert(self.validators.end(), b.validators.begin(), b.validators.end());
        return self;
    }

    /// Call to give the number of arguments expected on cli
    Combiner operator() (int n) const {
        Combiner self = *this;
        self.num = n;
        return self;
    }
    /// Call to give a validator
    Combiner operator() (std::function<bool(std::string)> func) const {
        Combiner self = *this;
        self.validators.push_back(func);
        return self;
    }
};

/// Check for an existing file
bool _ExistingFile(std::string filename) {
//    std::fstream f(name.c_str());
//    return f.good();
//    Fastest way according to http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
    struct stat buffer;   
    return (stat(filename.c_str(), &buffer) == 0); 
}

/// Check for an existing directory
bool _ExistingDirectory(std::string filename) {
    struct stat buffer;   
    if(stat(filename.c_str(), &buffer) == 0 && (buffer.st_mode & S_IFDIR) )
        return true;
    return false;
}

/// Check for a non-existing path
bool _NonexistentPath(std::string filename) {
    struct stat buffer;
    return stat(filename.c_str(), &buffer) != 0;
}




}



// Defines for common Combiners (don't use combiners directly)

const detail::Combiner Nothing    {0,  false, false, {}};
const detail::Combiner Required   {1,  true,  false, {}};
const detail::Combiner Default    {1,  false, true,  {}};
const detail::Combiner Args       {-1, false, false, {}};
const detail::Combiner Validators {1,  false, false, {}};

// Warning about using these validators:
// The files could be added/deleted after the validation. This is not common,
// but if this is a possibility, check the file you open afterwards
const detail::Combiner ExistingFile      {1, false, false, {detail::_ExistingFile}};
const detail::Combiner ExistingDirectory {1, false, false, {detail::_ExistingDirectory}};
const detail::Combiner NonexistentPath   {1, false, false, {detail::_NonexistentPath}};


}
