#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include "CLI/StringTools.hpp"
#include "CLI/TypeTools.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <string>

// C standard library
// Only needed for existence checking
// Could be swapped for filesystem in C++17
#include <sys/stat.h>
#include <sys/types.h>

namespace CLI {

class Option;

/// @defgroup validator_group Validators

/// @brief Some validators that are provided
///
/// These are simple `std::string(const std::string&)` validators that are useful. They return
/// a string if the validation fails. A custom struct is provided, as well, with the same user
/// semantics, but with the ability to provide a new type name.
/// @{

///
class Validator {
    friend Option;

  protected:
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

    /// create a validator that fails when a given validator succeeds
    Validator operator!() const {
        Validator newval;
        newval.tname = "NOT " + tname;

        std::string failString = "check " + tname + " succeeded improperly";
        // Give references (will make a copy in lambda function)
        const std::function<std::string(std::string & res)> &f1 = func;

        newval.func = [f1, failString](std::string &test) -> std::string {
            std::string s1 = f1(test);
            if(s1.empty())
                return failString;
            else
                return std::string();
        };
        return newval;
    }
};

// The implementation of the built in validators is using the Validator class;
// the user is only expected to use the const (static) versions (since there's no setup).
// Therefore, this is in detail.
namespace detail {

/// Check for an existing file (returns error message if check fails)
class ExistingFileValidator : public Validator {
  public:
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
class ExistingDirectoryValidator : public Validator {
  public:
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
class ExistingPathValidator : public Validator {
  public:
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
class NonexistentPathValidator : public Validator {
  public:
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
class IPV4Validator : public Validator {
  public:
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
class PositiveNumber : public Validator {
  public:
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
class Range : public Validator {
  public:
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

namespace detail {
template <typename T, enable_if_t<is_copyable_ptr<T>::value, detail::enabler> = detail::dummy>
auto smart_deref(T value) -> decltype(*value) {
    return *value;
}

template <typename T, enable_if_t<!is_copyable_ptr<T>::value, detail::enabler> = detail::dummy> T smart_deref(T value) {
    return value;
}
} // namespace detail

/// Verify items are in a set
class IsMember : public Validator {
  public:
    using filter_fn_t = std::function<std::string(std::string)>;

    /// This allows in-place construction using an initializer list
    template <typename T, typename... Args>
    explicit IsMember(std::initializer_list<T> values, Args &&... args)
        : IsMember(std::vector<T>(values), std::forward<Args>(args)...) {}

    /// This checks to see if an item is in a set (empty function)
    template <typename T>
    explicit IsMember(T set)
        : IsMember(std::move(set),
                   std::function<typename IsMemberType<typename element_value_type<T>::type>::type(
                       typename IsMemberType<typename element_value_type<T>::type>::type)>()) {}

    /// This checks to see if an item is in a set: pointer or copy version. You can pass in a function that will filter
    /// both sides of the comparison before computing the comparison.
    template <typename T, typename F> explicit IsMember(T set, F filter_function) {
        // Get the type of the contained item - requires a container have ::value_type
        using item_t = typename element_value_type<T>::type;
        using local_item_t = typename IsMemberType<item_t>::type;

        // Make a local copy of the filter function, using a std::function if not one already
        std::function<local_item_t(local_item_t)> filter_fn = filter_function;

        // This is the type name for help, it will take the current version of the set contents
        tname_function = [set]() {
            std::stringstream out;
            out << detail::type_name<item_t>() << " in {" << detail::join(detail::smart_deref(set), ",") << "}";
            return out.str();
        };

        // This is the function that validates
        // It stores a copy of the set pointer-like, so shared_ptr will stay alive
        func = [set, filter_fn](std::string &input) {
            for(const item_t &v : detail::smart_deref(set)) {
                local_item_t a = v;
                local_item_t b;
                if(!detail::lexical_cast(input, b))
                    throw ValidationError(input); // name is added later

                // The filter function might be empty, so don't filter if it is.
                if(filter_fn) {
                    a = filter_fn(a);
                    b = filter_fn(b);
                }

                if(a == b) {
                    // Make sure the version in the input string is identical to the one in the set
                    // Requires std::stringstream << be supported on T.
                    if(filter_fn) {
                        std::stringstream out;
                        out << v;
                        input = out.str();
                    }

                    // Return empty error string (success)
                    return std::string();
                }
            }

            // If you reach this point, the result was not found
            return input + " not in {" + detail::join(detail::smart_deref(set), ",") + "}";
        };
    }

    /// You can pass in as many filter functions as you like, they nest
    template <typename T, typename... Args>
    IsMember(T set, filter_fn_t filter_fn_1, filter_fn_t filter_fn_2, Args &&... other)
        : IsMember(std::move(set),
                   [filter_fn_1, filter_fn_2](std::string a) { return filter_fn_2(filter_fn_1(a)); },
                   other...) {}
};
template <typename T> using TransformPairs = std::vector<std::pair<std::string, T>>;

/// translate named items to other or a value set
class Transformer : public Validator {
  public:
    using filter_fn_t = std::function<std::string(std::string)>;

    /// This allows in-place construction
    template <typename... Args>
    explicit Transformer(std::initializer_list<std::pair<std::string, std::string>> values, Args &&... args)
        : Transformer(TransformPairs<std::string>(values), std::forward<Args>(args)...) {}

    /// direct map of std::string to std::string
    explicit Transformer(TransformPairs<std::string> mapping) : Transformer(std::move(mapping), filter_fn_t()) {}
    /// direct map of std::string to the string representation of some other object
    template <typename T>
    explicit Transformer(TransformPairs<T> mapping) : Transformer(std::move(mapping), filter_fn_t()) {}

    explicit Transformer(TransformPairs<std::string> mapping, filter_fn_t filter_fn) {
        // This is the type name for help, it will take the current version of the set contents
        tname_function = [mapping]() {
            std::stringstream out;
            out << "{"
                << detail::join(mapping,
                                [](const std::pair<std::string, std::string> &string_pair) {
                                    return std::string("\"") + string_pair.first + "\"->" + string_pair.second;
                                },
                                ",")
                << "}";
            return out.str();
        };

        func = [mapping, filter_fn](std::string &input) {
            for(const auto &ip : mapping) {
                // use a local const reference to keep alive the result of filter_fn
                const std::string &a = (filter_fn) ? filter_fn(ip.first) : ip.first;
                const std::string &b = (filter_fn) ? filter_fn(input) : input;
                if(a == b) {
                    input = ip.second;
                }
            }
            return std::string();
        };
    }

    template <typename T> explicit Transformer(TransformPairs<T> mapping, filter_fn_t filter_fn) {
        // This is the type name for help, it will take the current version of the set contents
        tname_function = [mapping]() {
            std::stringstream out;
            out << "{"
                << detail::join(mapping,
                                [](const std::pair<std::string, T> &string_pair) {
                                    std::stringstream out_mapping;
                                    out_mapping << '\"' << string_pair.first << "\"->" << string_pair.second;
                                    return out_mapping.str();
                                },
                                ",")
                << "}";
            return out.str();
        };

        func = [mapping, filter_fn](std::string &input) {
            for(const auto &ip : mapping) {
                // use a local const reference to keep alive the result of filter_fn
                const std::string &a = (filter_fn) ? filter_fn(ip.first) : ip.first;
                const std::string &b = (filter_fn) ? filter_fn(input) : input;
                if(a == b) {
                    std::stringstream out;
                    out << ip.second;
                    input = out.str();
                }
            }
            return std::string();
        };
    }

    /// You can pass in as many filter functions as you like, they nest
    template <typename T, typename... Args>
    Transformer(T mapping, filter_fn_t filter_fn_1, filter_fn_t filter_fn_2, Args &&... other)
        : Transformer(std::move(mapping),
                      [filter_fn_1, filter_fn_2](std::string a) { return filter_fn_2(filter_fn_1(a)); },
                      other...) {}
};

/// translate named items to other or a value set
class CheckedTransformer : public Validator {
  public:
    using filter_fn_t = std::function<std::string(std::string)>;

    /// This allows in-place construction
    template <typename... Args>
    explicit CheckedTransformer(std::initializer_list<std::pair<std::string, std::string>> values, Args &&... args)
        : CheckedTransformer(TransformPairs<std::string>(values), std::forward<Args>(args)...) {}

    /// direct map of std::string to std::string
    explicit CheckedTransformer(TransformPairs<std::string> mapping)
        : CheckedTransformer(std::move(mapping), filter_fn_t()) {}

    /// direct map of std::string to the string representation of some other object
    template <typename T>
    explicit CheckedTransformer(TransformPairs<T> mapping) : CheckedTransformer(std::move(mapping), filter_fn_t()) {}

    explicit CheckedTransformer(TransformPairs<std::string> mapping, filter_fn_t filter_fn) {
        // This is the type name for help, it will take the current version of the set contents
        std::stringstream out;
        out << "{"
            << detail::join(mapping,
                            [](const std::pair<std::string, std::string> &string_pair) {
                                return std::string("\"") + string_pair.first + "\"->" + string_pair.second;
                            },
                            ",")
            << ","
            << detail::join(mapping,
                            [](const std::pair<std::string, std::string> &string_pair) { return string_pair.second; },
                            ",")
            << "}";

        std::string value_string = out.str();
        tname = "value in " + value_string;

        func = [=](std::string &input) {
            for(const auto &ip : mapping) {
                // use a local const reference to keep alive the result of filter_fn
                const std::string &a = (filter_fn) ? filter_fn(ip.first) : ip.first;
                const std::string &b = (filter_fn) ? filter_fn(input) : input;
                if(a == b) {
                    input = ip.second;
                    return std::string();
                } else if(input == ip.second) {
                    return std::string();
                }
            }
            return input + " not in " + value_string;
        };
    }

    template <typename T> explicit CheckedTransformer(TransformPairs<T> mapping, filter_fn_t filter_fn) {
        std::vector<std::string> possible_outputs;
        possible_outputs.reserve(mapping.size());
        for(auto &ip : mapping) {
            std::stringstream out;
            out << ip.second;
            possible_outputs.emplace_back(out.str());
        }

        std::stringstream out;
        out << "{"
            << detail::join(mapping,
                            [](const std::pair<std::string, T> &string_pair) {
                                std::stringstream out_mapping;
                                out_mapping << '\"' << string_pair.first << "\"->" << string_pair.second;
                                return out_mapping.str();
                            },
                            ",")
            << "," << detail::join(possible_outputs, ",") << "}";
        std::string value_string = out.str();

        tname = std::string("value in ") + value_string;

        func = [=](std::string &input) {
            for(const auto &ip : mapping) {
                // use a local const reference to keep alive the result of filter_fn
                const std::string &a = (filter_fn) ? filter_fn(ip.first) : ip.first;
                const std::string &b = (filter_fn) ? filter_fn(input) : input;
                if(a == b) {
                    std::stringstream out;
                    out << ip.second;
                    input = out.str();
                    return std::string();
                }
            }
            for(const auto &ip : possible_outputs) {
                if(ip == input) {
                    return std::string();
                }
            }
            return input + " not in " + value_string;
        };
    }

    /// You can pass in as many filter functions as you like, they nest
    template <typename T, typename... Args>
    CheckedTransformer(T mapping, filter_fn_t filter_fn_1, filter_fn_t filter_fn_2, Args &&... other)
        : CheckedTransformer(std::move(mapping),
                             [filter_fn_1, filter_fn_2](std::string a) { return filter_fn_2(filter_fn_1(a)); },
                             other...) {}
}; // namespace CLI

/// Helper function to allow ignore_case to be passed to IsMember or Transform
inline std::string ignore_case(std::string item) { return detail::to_lower(item); }

/// Helper function to allow ignore_underscore to be passed to IsMember or Transform
inline std::string ignore_underscore(std::string item) { return detail::remove_underscore(item); }

/// Helper function to allow checks to ignore spaces to be passed to IsMember or Transform
inline std::string ignore_space(std::string item) {
    item.erase(std::remove(std::begin(item), std::end(item), ' '), std::end(item));
    return item;
}

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
