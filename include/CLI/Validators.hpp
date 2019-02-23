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

    /// Combining validators is a new validator. Type comes from left validator if function, otherwise only set if the
    /// same.
    Validator operator&(const Validator &other) const {
        Validator newval;
        newval.tname = (tname == other.tname ? tname : "");
        newval.tname_function = tname_function;

        // Give references (will make a copy in lambda function)
        const std::function<std::string(std::string & filename)> &f1 = func;
        const std::function<std::string(std::string & filename)> &f2 = other.func;

        newval.func = [f1, f2](std::string &input) {
            std::string s1 = f1(input);
            std::string s2 = f2(input);
            if(!s1.empty() && !s2.empty())
                return s1 + " AND " + s2;
            else
                return s1 + s2;
        };
        return newval;
    }

    /// Combining validators is a new validator. Type comes from left validator if function, otherwise only set if the
    /// same.
    Validator operator|(const Validator &other) const {
        Validator newval;
        newval.tname = (tname == other.tname ? tname : "");
        newval.tname_function = tname_function;

        // Give references (will make a copy in lambda function)
        const std::function<std::string(std::string &)> &f1 = func;
        const std::function<std::string(std::string &)> &f2 = other.func;

        newval.func = [f1, f2](std::string &input) {
            std::string s1 = f1(input);
            std::string s2 = f2(input);
            if(s1.empty() || s2.empty())
                return std::string();
            else
                return s1 + " OR " + s2;
        };
        return newval;
    }

    /// Create a validator that fails when a given validator succeeds
    Validator operator!() const {
        Validator newval;
        std::string typestring = tname;
        if(tname.empty()) {
            typestring = tname_function();
        }
        newval.tname = "NOT " + typestring;

        std::string failString = "check " + typestring + " succeeded improperly";
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
/// Generate a string representation of a set
template <typename T> std::string generate_set(const T &set) {
    using element_t = typename detail::element_type<T>::type;
    using iteration_type_t = typename detail::pair_adaptor<element_t>::value_type; // the type of the object pair
    std::string out(1, '{');
    out.append(detail::join(detail::smart_deref(set),
                            [](const iteration_type_t &v) { return detail::pair_adaptor<element_t>::first(v); },
                            ","));
    out.push_back('}');
    return out;
}

/// Generate a string representation of a map
template <typename T> std::string generate_map(const T &map) {
    using element_t = typename detail::element_type<T>::type;
    using iteration_type_t = typename detail::pair_adaptor<element_t>::value_type; // the type of the object pair
    std::string out(1, '{');
    out.append(detail::join(detail::smart_deref(map),
                            [](const iteration_type_t &v) {
                                return std::string("\"") +
                                       detail::as_string(detail::pair_adaptor<element_t>::first(v)) + "\"->" +
                                       detail::as_string(detail::pair_adaptor<element_t>::second(v));
                            },
                            ","));
    out.push_back('}');
    return out;
}

template <typename> struct sfinae_true : std::true_type {};
/// Function to check for the existence of a member find function
template <typename T, typename V>
static auto test_find(int) -> sfinae_true<decltype(std::declval<T>().find(std::declval<V>()))>;
template <typename, typename V> static auto test_find(long) -> std::false_type;
} // namespace detail

template <typename T, typename V> struct has_find : decltype(detail::test_find<T, V>(0)) {};

/// A search function
template <typename T, typename V>
std::pair<bool, std::pair<V, typename detail::pair_adaptor<typename detail::element_type<T>::type>::second_type>>
search(const T &set, const V &val) {
    using element_t = typename detail::element_type<T>::type;
    for(const auto &v : set) {
        V a = detail::pair_adaptor<element_t>::first(v);
        if(a == val) {
            // Return empty error string (success)
            return {true, {a, detail::pair_adaptor<element_t>::second(v)}};
        }
    }
    return {false, {}};
}

/// A search function that uses the built in find function
template <typename T, typename V, enable_if_t<has_find<T, V>::value, detail::enabler> = detail::dummy>
std::pair<bool, std::pair<V, typename detail::pair_adaptor<typename detail::element_type<T>::type>::second_type>>
search(const T &set, const V &val) {}

/// A search function with a filter function
template <typename T, typename V, typename F>
std::pair<bool, std::pair<V, typename detail::pair_adaptor<typename detail::element_type<T>::type>::second_type>>
search(const T &set, const V &val, F &filter_function) {
    // do the potentially faster first
    auto res = search(set, val);
    if((res.first) || (!(filter_function))) {
        return res;
    }
    // now do a slower search with a translate for each search element
    using element_t = typename detail::element_type<T>::type;
    for(const auto &v : set) {
        V a = detail::pair_adaptor<element_t>::first(v);
        a = filter_function(a);
        if(a == val) {
            // Return empty error string (success)
            return {true, {detail::pair_adaptor<element_t>::first(v), detail::pair_adaptor<element_t>::second(v)}};
        }
    }
    return {false, {}};
}

/// Verify items are in a set
class IsMember : public Validator {
  public:
    using filter_fn_t = std::function<std::string(std::string)>;

    /// This allows in-place construction using an initializer list
    template <typename T, typename... Args>
    explicit IsMember(std::initializer_list<T> values, Args &&... args)
        : IsMember(std::vector<T>(values), std::forward<Args>(args)...) {}

    /// This checks to see if an item is in a set (empty function)
    template <typename T> explicit IsMember(T set) : IsMember(std::move(set), nullptr) {}

    /// This checks to see if an item is in a set: pointer or copy version. You can pass in a function that will filter
    /// both sides of the comparison before computing the comparison.
    template <typename T, typename F> explicit IsMember(T set, F filter_function) {

        // Get the type of the contained item - requires a container have ::value_type
        // if the type does not have first_type and second_type, these are both value_type
        using element_t = typename detail::element_type<T>::type;            // Removes (smart) pointers if needed
        using item_t = typename detail::pair_adaptor<element_t>::first_type; // Is value_type if not a map

        using local_item_t = typename IsMemberType<item_t>::type; // This will convert bad types to good ones
                                                                  // (const char * to std::string)

        // Make a local copy of the filter function, using a std::function if not one already
        std::function<local_item_t(local_item_t)> filter_fn = filter_function;

        // This is the type name for help, it will take the current version of the set contents
        tname_function = [set]() { return detail::generate_set(detail::smart_deref(set)); };

        // This is the function that validates
        // It stores a copy of the set pointer-like, so shared_ptr will stay alive
        func = [set, filter_fn](std::string &input) {
            local_item_t b;
            if(!detail::lexical_cast(input, b)) {
                throw ValidationError(input); // name is added later
            }
            if(filter_fn) {
                b = filter_fn(b);
            }
            for(const auto &v : detail::smart_deref(set)) {
                local_item_t a = detail::pair_adaptor<element_t>::first(v);
                // The filter function might be empty, so don't filter if it is.
                if(filter_fn) {
                    a = filter_fn(a);
                }

                if(a == b) {
                    // Make sure the version in the input string is identical to the one in the set
                    if(filter_fn || detail::pair_adaptor<element_t>::value) {
                        input = detail::as_string(detail::pair_adaptor<element_t>::first(v));
                    }

                    // Return empty error string (success)
                    return std::string();
                }
            }

            // If you reach this point, the result was not found
            std::string out(" not in ");
            out += detail::generate_set(detail::smart_deref(set));
            return out;
        };
    }

    /// You can pass in as many filter functions as you like, they nest (string only currently)
    template <typename T, typename... Args>
    IsMember(T set, filter_fn_t filter_fn_1, filter_fn_t filter_fn_2, Args &&... other)
        : IsMember(std::move(set),
                   [filter_fn_1, filter_fn_2](std::string a) { return filter_fn_2(filter_fn_1(a)); },
                   other...) {}
};

/// definition of the default transformation object
template <typename T> using TransformPairs = std::vector<std::pair<std::string, T>>;

/// Translate named items to other or a value set
class Transformer : public Validator {
  public:
    using filter_fn_t = std::function<std::string(std::string)>;

    /// This allows in-place construction
    template <typename... Args>
    explicit Transformer(std::initializer_list<std::pair<std::string, std::string>> values, Args &&... args)
        : Transformer(TransformPairs<std::string>(values), std::forward<Args>(args)...) {}

    /// direct map of std::string to std::string
    template <typename T> explicit Transformer(T mapping) : Transformer(std::move(mapping), nullptr) {}

    /// This checks to see if an item is in a set: pointer or copy version. You can pass in a function that will filter
    /// both sides of the comparison before computing the comparison.
    template <typename T, typename F> explicit Transformer(T mapping, F filter_function) {

        static_assert(detail::pair_adaptor<typename detail::element_type<T>::type>::value,
                      "mapping must produce value pairs");
        // Get the type of the contained item - requires a container have ::value_type
        // if the type does not have first_type and second_type, these are both value_type
        using element_t = typename detail::element_type<T>::type;            // Removes (smart) pointers if needed
        using item_t = typename detail::pair_adaptor<element_t>::first_type; // Is value_type if not a map
        using local_item_t = typename IsMemberType<item_t>::type;            // This will convert bad types to good ones
                                                                             // (const char * to std::string)

        // Make a local copy of the filter function, using a std::function if not one already
        std::function<local_item_t(local_item_t)> filter_fn = filter_function;

        // This is the type name for help, it will take the current version of the set contents
        tname_function = [mapping]() { return detail::generate_map(detail::smart_deref(mapping)); };

        func = [mapping, filter_fn](std::string &input) {
            local_item_t b;
            if(!detail::lexical_cast(input, b)) {
                return std::string();
                // there is no possible way we can match anything in the mapping if we can't convert so just return
            }
            if(filter_fn) {
                b = filter_fn(b);
            }

            for(const auto &v : detail::smart_deref(mapping)) {
                local_item_t a = detail::pair_adaptor<element_t>::first(v);

                // The filter function might be empty, so don't filter if it is.
                if(filter_fn) {
                    a = filter_fn(a);
                }

                if(a == b) {
                    // transform the input string
                    // Requires std::stringstream << be supported on T
                    input = detail::as_string(detail::pair_adaptor<element_t>::second(v));
                    // Return empty error string (success)
                    return std::string{};
                }
            }

            return std::string{};
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
    template <typename T> explicit CheckedTransformer(T mapping) : CheckedTransformer(std::move(mapping), nullptr) {}

    /// This checks to see if an item is in a set: pointer or copy version. You can pass in a function that will filter
    /// both sides of the comparison before computing the comparison.
    template <typename T, typename F> explicit CheckedTransformer(T mapping, F filter_function) {

        static_assert(detail::pair_adaptor<typename detail::element_type<T>::type>::value,
                      "mapping must produce value pairs");
        // Get the type of the contained item - requires a container have ::value_type
        // if the type does not have first_type and second_type, these are both value_type
        using element_t = typename detail::element_type<T>::type;            // Removes (smart) pointers if needed
        using item_t = typename detail::pair_adaptor<element_t>::first_type; // Is value_type if not a map
        using local_item_t = typename IsMemberType<item_t>::type;            // This will convert bad types to good ones
                                                                             // (const char * to std::string)
        using iteration_type_t = typename detail::pair_adaptor<element_t>::value_type; // the type of the object pair //
                                                                                       // the type of the object pair

        // Make a local copy of the filter function, using a std::function if not one already
        std::function<local_item_t(local_item_t)> filter_fn = filter_function;

        auto tfunc = [mapping]() {
            std::string out("value in ");
            out += detail::generate_map(detail::smart_deref(mapping)) + " OR {";
            out += detail::join(
                detail::smart_deref(mapping),
                [](const iteration_type_t &v) { return detail::as_string(detail::pair_adaptor<element_t>::second(v)); },
                ",");
            out.push_back('}');
            return out;
        };
        // assign the tname_function
        tname_function = tfunc;

        func = [mapping, tfunc, filter_fn](std::string &input) {
            local_item_t b;
            bool converted = detail::lexical_cast(input, b);
            if(converted) {
                if(filter_fn) {
                    b = filter_fn(b);
                }

                for(const auto &v : detail::smart_deref(mapping)) {
                    local_item_t a = detail::pair_adaptor<element_t>::first(v);

                    // The filter function might be empty, so don't filter if it is.
                    if(filter_fn) {
                        a = filter_fn(a);
                    }

                    if(a == b) {
                        // transform the input string
                        // Requires std::stringstream << be supported on T
                        input = detail::as_string(detail::pair_adaptor<element_t>::second(v));
                        // Return empty error string (success)
                        return std::string{};
                    }
                }
            }
            for(const auto &v : detail::smart_deref(mapping)) {
                auto output_string = detail::as_string(detail::pair_adaptor<element_t>::second(v));
                if(output_string == input) {
                    return std::string();
                }
            }

            return "Check " + input + " " + tfunc() + " FAILED";
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
/// the return value contains is a pair with the first argument containing the program name and the second
/// everything else.
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
