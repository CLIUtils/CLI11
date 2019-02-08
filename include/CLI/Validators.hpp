#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include "CLI/StringTools.hpp"
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

    /// This is the type function, if empty the tname will be used
    std::function<std::string()> tname_function;

    /// This it the base function that is to be called.
    /// Returns a string error message if validation fails.
    std::function<std::string(std::string &)> func;

  public:
    /// This is the required operator for a validator - provided to help
    /// users (CLI11 uses the member `func` directly)
    std::string operator()(std::string &str) const { return func(str); };

    /// This is the required operator for a validator - provided to help
    /// users (CLI11 uses the member `func` directly)
    std::string operator()(const std::string &str) const {
        std::string value = str;
        return func(value);
    };

    /// Combining validators is a new validator
    Validator operator&(const Validator &other) const {
        Validator newval;
        newval.tname = (tname == other.tname ? tname : "");

        // Give references (will make a copy in lambda function)
        const std::function<std::string(std::string & filename)> &f1 = func;
        const std::function<std::string(std::string & filename)> &f2 = other.func;

        newval.func = [f1, f2](std::string &filename) {
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
        const std::function<std::string(std::string & filename)> &f1 = func;
        const std::function<std::string(std::string & filename)> &f2 = other.func;

        newval.func = [f1, f2](std::string &filename) {
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
        func = [](std::string &filename) {
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
        func = [](std::string &filename) {
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
        func = [](std::string &filename) {
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
        func = [](std::string &filename) {
            struct stat buffer;
            bool exist = stat(filename.c_str(), &buffer) == 0;
            if(exist) {
                return "Path already exists: " + filename;
            }
            return std::string();
        };
    }
};

/// Validate the given string is a legal ipv4 address
struct IPV4Validator : public Validator {
    IPV4Validator() {
        tname = "IPV4";
        func = [](std::string &ip_addr) {
            auto result = CLI::detail::split(ip_addr, '.');
            if(result.size() != 4) {
                return "Invalid IPV4 address must have four parts " + ip_addr;
            }
            int num;
            bool retval = true;
            for(const auto &var : result) {
                retval &= detail::lexical_cast(var, num);
                if(!retval) {
                    return "Failed parsing number " + var;
                }
                if(num < 0 || num > 255) {
                    return "Each IP number must be between 0 and 255 " + var;
                }
            }
            return std::string();
        };
    }
};

/// Validate the argument is a number and greater than or equal to 0
struct PositiveNumber : public Validator {
    PositiveNumber() {
        tname = "POSITIVE";
        func = [](std::string &number_str) {
            int number;
            if(!detail::lexical_cast(number_str, number)) {
                return "Failed parsing number " + number_str;
            }
            if(number < 0) {
                return "Number less then 0 " + number_str;
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

/// Check for an IP4 address
const detail::IPV4Validator ValidIPV4;

/// Check for a positive number
const detail::PositiveNumber PositiveNumber;

/// Produce a range (factory). Min and max are inclusive.
struct Range : public Validator {
    /// This produces a range with min and max inclusive.
    ///
    /// Note that the constructor is templated, but the struct is not, so C++17 is not
    /// needed to provide nice syntax for Range(a,b).
    template <typename T> Range(T min, T max) {
        std::stringstream out;
        out << detail::type_name<T>() << " in [" << min << " - " << max << "]";

        tname = out.str();
        func = [min, max](std::string &input) {
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

/// Verify items are in a set
struct IsMember : public Validator {
    using filter_fn_t = std::function<std::string(std::string)>;

    /// This checks to see if an item is in a set: pointer version. The pointer-like must be copyable. (Normal pointers
    /// and shared pointers by default).
    ///
    /// Note that the constructor is templated, but the struct is not, so C++17 is not
    /// needed to provide nice syntax for IsMember(set).
    template <typename T, enable_if_t<is_copyable_ptr<T>::value, detail::enabler> = detail::dummy>
    explicit IsMember(T set,
                      std::function<typename std::decay<decltype(*set)>::type::value_type(
                          typename std::decay<decltype(*set)>::type::value_type)> filter_fn = {}) {
        using item_t = typename std::decay<decltype(*set)>::type::value_type;

        tname_function = [set]() {
            std::stringstream out;
            out << detail::type_name<item_t>() << " in {" << detail::join(*set, ",") << "}";
            return out.str();
        };

        func = [set, filter_fn](std::string &input) {
            auto result = std::find_if(std::begin(*set), std::end(*set), [filter_fn, input](item_t v) {
                item_t a = v;
                item_t b;
                if(!detail::lexical_cast(input, b))
                    throw ConversionError(input); // name is added later

                if(filter_fn) {
                    a = filter_fn(a);
                    b = filter_fn(b);
                }
                return a == b;
            });

            if(result == std::end(*set)) {
                return input + " not in {" + detail::join(*set, ",") + "}";
            } else {
                // Make sure the version in the input string is identical to the one in the set
                // Requires std::stringstream << be supported on T.
                std::stringstream out;
                out << *result;
                input = out.str();
                return std::string();
            }
        };
    }

    /// This checks to see if an item is in a set: non-pointer version.
    ///
    /// Internally copies into a shared pointer and sends it
    /// through the next constructor to avoid duplicating logic.
    template <typename T, enable_if_t<!is_copyable_ptr<T>::value, detail::enabler> = detail::dummy, typename... Args>
    explicit IsMember(T set, Args &&... other) : IsMember(std::make_shared<T>(set), other...) {}

    /// Shortcut to allow inplace initilizer lists to be used as sets.
    ///
    /// Vector used internally to ensure order preservation.
    template <typename... Args>
    explicit IsMember(std::initializer_list<std::string> items, Args &&... other)
        : IsMember(std::vector<std::string>(items), other...) {}

    /// You can pass in as many filter functions as you like, they nest
    template <typename T, typename... Args>
    IsMember(T set, filter_fn_t filter_fn_1, filter_fn_t filter_fn_2, Args &&... other)
        : IsMember(
              set, [filter_fn_1, filter_fn_2](std::string a) { return filter_fn_2(filter_fn_1(a)); }, other...) {}
};

/// Helper function to allow ignore_case to be passed to IsMember
inline std::string ignore_case(std::string item) { return detail::to_lower(item); }

/// Helper function to allow ignore_underscore to be passed to IsMember
inline std::string ignore_underscore(std::string item) { return detail::remove_underscore(item); }

namespace detail {
/// Split a string into a program name and command line arguments
/// the string is assumed to contain a file name followed by other arguments
/// the return value contains is a pair with the first argument containing the program name and the second everything
/// else.
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
