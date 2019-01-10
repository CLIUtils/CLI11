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
/// These are simple `std::string(const std::string&)` validators that are useful. They return
/// a string if the validation fails. A custom struct is provided, as well, with the same user
/// semantics, but with the ability to provide a new type name.
/// @{

///
struct Validator {
    /// This is the type name, if empty the type name will not be changed
    std::string tname;

    /// This it the base function that is to be called.
    /// Returns a string error message if validation fails.
    std::function<std::string(const std::string &)> func;

    /// This is the required operator for a validator - provided to help
    /// users (CLI11 uses the member `func` directly)
    std::string operator()(const std::string &str) const { return func(str); };

    /// Combining validators is a new validator
    Validator operator&(const Validator &other) const {
        Validator newval;
        newval.tname = (tname == other.tname ? tname : "");

        // Give references (will make a copy in lambda function)
        const std::function<std::string(const std::string &filename)> &f1 = func;
        const std::function<std::string(const std::string &filename)> &f2 = other.func;

        newval.func = [f1, f2](const std::string &filename) {
            std::string s1 = f1(filename);
            std::string s2 = f2(filename);
            if(!s1.empty() && !s2.empty())
                return s1 + " & " + s2;
            else
                return s1 + s2;
        };
        return newval;
    }

    /// Combining validators is a new validator
    Validator operator|(const Validator &other) const {
        Validator newval;
        newval.tname = (tname == other.tname ? tname : "");

        // Give references (will make a copy in lambda function)
        const std::function<std::string(const std::string &filename)> &f1 = func;
        const std::function<std::string(const std::string &filename)> &f2 = other.func;

        newval.func = [f1, f2](const std::string &filename) {
            std::string s1 = f1(filename);
            std::string s2 = f2(filename);
            if(s1.empty() || s2.empty())
                return std::string();
            else
                return s1 + " & " + s2;
        };
        return newval;
    }
};

// The implementation of the built in validators is using the Validator class;
// the user is only expected to use the const (static) versions (since there's no setup).
// Therefore, this is in detail.
namespace detail {

/// Check for an existing file (returns error message if check fails)
struct ExistingFileValidator : public Validator {
    ExistingFileValidator() {
        tname = "FILE";
        func = [](const std::string &filename) {
            struct stat buffer;
            bool exist = stat(filename.c_str(), &buffer) == 0;
            bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
            if(!exist) {
                return "File does not exist: " + filename;
            } else if(is_dir) {
                return "File is actually a directory: " + filename;
            }
            return std::string();
        };
    }
};

/// Check for an existing directory (returns error message if check fails)
struct ExistingDirectoryValidator : public Validator {
    ExistingDirectoryValidator() {
        tname = "DIR";
        func = [](const std::string &filename) {
            struct stat buffer;
            bool exist = stat(filename.c_str(), &buffer) == 0;
            bool is_dir = (buffer.st_mode & S_IFDIR) != 0;
            if(!exist) {
                return "Directory does not exist: " + filename;
            } else if(!is_dir) {
                return "Directory is actually a file: " + filename;
            }
            return std::string();
        };
    }
};

/// Check for an existing path
struct ExistingPathValidator : public Validator {
    ExistingPathValidator() {
        tname = "PATH";
        func = [](const std::string &filename) {
            struct stat buffer;
            bool const exist = stat(filename.c_str(), &buffer) == 0;
            if(!exist) {
                return "Path does not exist: " + filename;
            }
            return std::string();
        };
    }
};

/// Check for an non-existing path
struct NonexistentPathValidator : public Validator {
    NonexistentPathValidator() {
        tname = "PATH";
        func = [](const std::string &filename) {
            struct stat buffer;
            bool exist = stat(filename.c_str(), &buffer) == 0;
            if(exist) {
                return "Path already exists: " + filename;
            }
            return std::string();
        };
    }
};
} // namespace detail

// Static is not needed here, because global const implies static.

/// Check for existing file (returns error message if check fails)
const detail::ExistingFileValidator ExistingFile;

/// Check for an existing directory (returns error message if check fails)
const detail::ExistingDirectoryValidator ExistingDirectory;

/// Check for an existing path
const detail::ExistingPathValidator ExistingPath;

/// Check for an non-existing path
const detail::NonexistentPathValidator NonexistentPath;

///  Produce a range (factory). Min and max are inclusive.
struct Range : public Validator {
    /// This produces a range with min and max inclusive.
    ///
    /// Note that the constructor is templated, but the struct is not, so C++17 is not
    /// needed to provide nice syntax for Range(a,b).
    template <typename T> Range(T min, T max) {
        std::stringstream out;
        out << detail::type_name<T>() << " in [" << min << " - " << max << "]";

        tname = out.str();
        func = [min, max](std::string input) {
            T val;
            detail::lexical_cast(input, val);
            if(val < min || val > max)
                return "Value " + input + " not in range " + std::to_string(min) + " to " + std::to_string(max);

            return std::string();
        };
    }

    /// Range of one value is 0 to value
    template <typename T> explicit Range(T max) : Range(static_cast<T>(0), max) {}
};

namespace detail {
/// split a string into a program name and command line arguments
/// the string is assumed to contain a file name followed by other arguments
/// the return value contains is a pair with the first argument containing the program name and the second everything
/// else
inline std::pair<std::string, std::string> split_program_name(std::string commandline) {
    // try to determine the programName
    std::pair<std::string, std::string> vals;
    trim(commandline);
    auto esp = commandline.find_first_of(' ', 1);
    while(!ExistingFile(commandline.substr(0, esp)).empty()) {
        esp = commandline.find_first_of(' ', esp + 1);
        if(esp == std::string::npos) {
            // if we have reached the end and haven't found a valid file just assume the first argument is the
            // program name
            esp = commandline.find_first_of(' ', 1);
            break;
        }
    }
    vals.first = commandline.substr(0, esp);
    rtrim(vals.first);
    // strip the program name
    vals.second = (esp != std::string::npos) ? commandline.substr(esp + 1) : std::string{};
    ltrim(vals.second);
    return vals;
}
} // namespace detail
/// @}

} // namespace CLI
