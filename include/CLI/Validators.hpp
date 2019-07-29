#pragma once
// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include "CLI/StringTools.hpp"
#include "CLI/TypeTools.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
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
  protected:
    /// This is the description function, if empty the description_ will be used
    std::function<std::string()> desc_function_{[]() { return std::string{}; }};

    /// This it the base function that is to be called.
    /// Returns a string error message if validation fails.
    std::function<std::string(std::string &)> func_{[](std::string &) { return std::string{}; }};
    /// The name for search purposes of the Validator
    std::string name_;
    /// Enable for Validator to allow it to be disabled if need be
    bool active_{true};
    /// specify that a validator should not modify the input
    bool non_modifying_{false};

  public:
    Validator() = default;
    /// Construct a Validator with just the description string
    explicit Validator(std::string validator_desc) : desc_function_([validator_desc]() { return validator_desc; }) {}
    // Construct Validator from basic information
    Validator(std::function<std::string(std::string &)> op, std::string validator_desc, std::string validator_name = "")
        : desc_function_([validator_desc]() { return validator_desc; }), func_(std::move(op)),
          name_(std::move(validator_name)) {}
    /// Set the Validator operation function
    Validator &operation(std::function<std::string(std::string &)> op) {
        func_ = std::move(op);
        return *this;
    }
    /// This is the required operator for a Validator - provided to help
    /// users (CLI11 uses the member `func` directly)
    std::string operator()(std::string &str) const {
        std::string retstring;
        if(active_) {
            if(non_modifying_) {
                std::string value = str;
                retstring = func_(value);
            } else {
                retstring = func_(str);
            }
        }
        return retstring;
    };

    /// This is the required operator for a Validator - provided to help
    /// users (CLI11 uses the member `func` directly)
    std::string operator()(const std::string &str) const {
        std::string value = str;
        return (active_) ? func_(value) : std::string{};
    };

    /// Specify the type string
    Validator &description(std::string validator_desc) {
        desc_function_ = [validator_desc]() { return validator_desc; };
        return *this;
    }
    /// Generate type description information for the Validator
    std::string get_description() const {
        if(active_) {
            return desc_function_();
        }
        return std::string{};
    }
    /// Specify the type string
    Validator &name(std::string validator_name) {
        name_ = std::move(validator_name);
        return *this;
    }
    /// Get the name of the Validator
    const std::string &get_name() const { return name_; }
    /// Specify whether the Validator is active or not
    Validator &active(bool active_val = true) {
        active_ = active_val;
        return *this;
    }

    /// Specify whether the Validator can be modifying or not
    Validator &non_modifying(bool no_modify = true) {
        non_modifying_ = no_modify;
        return *this;
    }

    /// Get a boolean if the validator is active
    bool get_active() const { return active_; }

    /// Get a boolean if the validator is allowed to modify the input returns true if it can modify the input
    bool get_modifying() const { return !non_modifying_; }

    /// Combining validators is a new validator. Type comes from left validator if function, otherwise only set if the
    /// same.
    Validator operator&(const Validator &other) const {
        Validator newval;

        newval._merge_description(*this, other, " AND ");

        // Give references (will make a copy in lambda function)
        const std::function<std::string(std::string & filename)> &f1 = func_;
        const std::function<std::string(std::string & filename)> &f2 = other.func_;

        newval.func_ = [f1, f2](std::string &input) {
            std::string s1 = f1(input);
            std::string s2 = f2(input);
            if(!s1.empty() && !s2.empty())
                return std::string("(") + s1 + ") AND (" + s2 + ")";
            else
                return s1 + s2;
        };

        newval.active_ = (active_ & other.active_);
        return newval;
    }

    /// Combining validators is a new validator. Type comes from left validator if function, otherwise only set if the
    /// same.
    Validator operator|(const Validator &other) const {
        Validator newval;

        newval._merge_description(*this, other, " OR ");

        // Give references (will make a copy in lambda function)
        const std::function<std::string(std::string &)> &f1 = func_;
        const std::function<std::string(std::string &)> &f2 = other.func_;

        newval.func_ = [f1, f2](std::string &input) {
            std::string s1 = f1(input);
            std::string s2 = f2(input);
            if(s1.empty() || s2.empty())
                return std::string();
            else
                return std::string("(") + s1 + ") OR (" + s2 + ")";
        };
        newval.active_ = (active_ & other.active_);
        return newval;
    }

    /// Create a validator that fails when a given validator succeeds
    Validator operator!() const {
        Validator newval;
        const std::function<std::string()> &dfunc1 = desc_function_;
        newval.desc_function_ = [dfunc1]() {
            auto str = dfunc1();
            return (!str.empty()) ? std::string("NOT ") + str : std::string{};
        };
        // Give references (will make a copy in lambda function)
        const std::function<std::string(std::string & res)> &f1 = func_;

        newval.func_ = [f1, dfunc1](std::string &test) -> std::string {
            std::string s1 = f1(test);
            if(s1.empty()) {
                return std::string("check ") + dfunc1() + " succeeded improperly";
            } else
                return std::string{};
        };
        newval.active_ = active_;
        return newval;
    }

  private:
    void _merge_description(const Validator &val1, const Validator &val2, const std::string &merger) {

        const std::function<std::string()> &dfunc1 = val1.desc_function_;
        const std::function<std::string()> &dfunc2 = val2.desc_function_;

        desc_function_ = [=]() {
            std::string f1 = dfunc1();
            std::string f2 = dfunc2();
            if((f1.empty()) || (f2.empty())) {
                return f1 + f2;
            }
            return std::string("(") + f1 + ")" + merger + "(" + f2 + ")";
        };
    }
};

/// Class wrapping some of the accessors of Validator
class CustomValidator : public Validator {
  public:
};
// The implementation of the built in validators is using the Validator class;
// the user is only expected to use the const (static) versions (since there's no setup).
// Therefore, this is in detail.
namespace detail {

/// Check for an existing file (returns error message if check fails)
class ExistingFileValidator : public Validator {
  public:
    ExistingFileValidator() : Validator("FILE") {
        func_ = [](std::string &filename) {
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
    ExistingDirectoryValidator() : Validator("DIR") {
        func_ = [](std::string &filename) {
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
    ExistingPathValidator() : Validator("PATH(existing)") {
        func_ = [](std::string &filename) {
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
    NonexistentPathValidator() : Validator("PATH(non-existing)") {
        func_ = [](std::string &filename) {
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
    IPV4Validator() : Validator("IPV4") {
        func_ = [](std::string &ip_addr) {
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
    PositiveNumber() : Validator("POSITIVE") {
        func_ = [](std::string &number_str) {
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

/// Validate the argument is a number and greater than or equal to 0
class Number : public Validator {
  public:
    Number() : Validator("NUMBER") {
        func_ = [](std::string &number_str) {
            double number;
            if(!detail::lexical_cast(number_str, number)) {
                return "Failed parsing as a number " + number_str;
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

/// Check for a number
const detail::Number Number;

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
        description(out.str());

        func_ = [min, max](std::string &input) {
            T val;
            bool converted = detail::lexical_cast(input, val);
            if((!converted) || (val < min || val > max))
                return "Value " + input + " not in range " + std::to_string(min) + " to " + std::to_string(max);

            return std::string();
        };
    }

    /// Range of one value is 0 to value
    template <typename T> explicit Range(T max) : Range(static_cast<T>(0), max) {}
};

/// Produce a bounded range (factory). Min and max are inclusive.
class Bound : public Validator {
  public:
    /// This bounds a value with min and max inclusive.
    ///
    /// Note that the constructor is templated, but the struct is not, so C++17 is not
    /// needed to provide nice syntax for Range(a,b).
    template <typename T> Bound(T min, T max) {
        std::stringstream out;
        out << detail::type_name<T>() << " bounded to [" << min << " - " << max << "]";
        description(out.str());

        func_ = [min, max](std::string &input) {
            T val;
            bool converted = detail::lexical_cast(input, val);
            if(!converted) {
                return "Value " + input + " could not be converted";
            }
            if(val < min)
                input = detail::to_string(min);
            else if(val > max)
                input = detail::to_string(max);

            return std::string{};
        };
    }

    /// Range of one value is 0 to value
    template <typename T> explicit Bound(T max) : Bound(static_cast<T>(0), max) {}
};

namespace detail {
template <typename T,
          enable_if_t<is_copyable_ptr<typename std::remove_reference<T>::type>::value, detail::enabler> = detail::dummy>
auto smart_deref(T value) -> decltype(*value) {
    return *value;
}

template <
    typename T,
    enable_if_t<!is_copyable_ptr<typename std::remove_reference<T>::type>::value, detail::enabler> = detail::dummy>
typename std::remove_reference<T>::type &smart_deref(T &value) {
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
template <typename T> std::string generate_map(const T &map, bool key_only = false) {
    using element_t = typename detail::element_type<T>::type;
    using iteration_type_t = typename detail::pair_adaptor<element_t>::value_type; // the type of the object pair
    std::string out(1, '{');
    out.append(detail::join(detail::smart_deref(map),
                            [key_only](const iteration_type_t &v) {
                                std::string res{detail::to_string(detail::pair_adaptor<element_t>::first(v))};

                                if(!key_only) {
                                    res.append("->");
                                    res += detail::to_string(detail::pair_adaptor<element_t>::second(v));
                                }
                                return res;
                            },
                            ","));
    out.push_back('}');
    return out;
}

template <typename> struct sfinae_true : std::true_type {};
/// Function to check for the existence of a member find function which presumably is more efficient than looping over
/// everything
template <typename T, typename V>
static auto test_find(int) -> sfinae_true<decltype(std::declval<T>().find(std::declval<V>()))>;
template <typename, typename V> static auto test_find(long) -> std::false_type;

template <typename T, typename V> struct has_find : decltype(test_find<T, V>(0)) {};

/// A search function
template <typename T, typename V, enable_if_t<!has_find<T, V>::value, detail::enabler> = detail::dummy>
auto search(const T &set, const V &val) -> std::pair<bool, decltype(std::begin(detail::smart_deref(set)))> {
    using element_t = typename detail::element_type<T>::type;
    auto &setref = detail::smart_deref(set);
    auto it = std::find_if(std::begin(setref), std::end(setref), [&val](decltype(*std::begin(setref)) v) {
        return (detail::pair_adaptor<element_t>::first(v) == val);
    });
    return {(it != std::end(setref)), it};
}

/// A search function that uses the built in find function
template <typename T, typename V, enable_if_t<has_find<T, V>::value, detail::enabler> = detail::dummy>
auto search(const T &set, const V &val) -> std::pair<bool, decltype(std::begin(detail::smart_deref(set)))> {
    auto &setref = detail::smart_deref(set);
    auto it = setref.find(val);
    return {(it != std::end(setref)), it};
}

/// A search function with a filter function
template <typename T, typename V>
auto search(const T &set, const V &val, const std::function<V(V)> &filter_function)
    -> std::pair<bool, decltype(std::begin(detail::smart_deref(set)))> {
    using element_t = typename detail::element_type<T>::type;
    // do the potentially faster first search
    auto res = search(set, val);
    if((res.first) || (!(filter_function))) {
        return res;
    }
    // if we haven't found it do the longer linear search with all the element translations
    auto &setref = detail::smart_deref(set);
    auto it = std::find_if(std::begin(setref), std::end(setref), [&](decltype(*std::begin(setref)) v) {
        V a{detail::pair_adaptor<element_t>::first(v)};
        a = filter_function(a);
        return (a == val);
    });
    return {(it != std::end(setref)), it};
}

// the following suggestion was made by Nikita Ofitserov(@himikof)
// done in templates to prevent compiler warnings on negation of unsigned numbers

/// Do a check for overflow on signed numbers
template <typename T>
inline typename std::enable_if<std::is_signed<T>::value, T>::type overflowCheck(const T &a, const T &b) {
    if((a > 0) == (b > 0)) {
        return ((std::numeric_limits<T>::max)() / (std::abs)(a) < (std::abs)(b));
    } else {
        return ((std::numeric_limits<T>::min)() / (std::abs)(a) > -(std::abs)(b));
    }
}
/// Do a check for overflow on unsigned numbers
template <typename T>
inline typename std::enable_if<!std::is_signed<T>::value, T>::type overflowCheck(const T &a, const T &b) {
    return ((std::numeric_limits<T>::max)() / a < b);
}

/// Performs a *= b; if it doesn't cause integer overflow. Returns false otherwise.
template <typename T> typename std::enable_if<std::is_integral<T>::value, bool>::type checked_multiply(T &a, T b) {
    if(a == 0 || b == 0 || a == 1 || b == 1) {
        a *= b;
        return true;
    }
    if(a == (std::numeric_limits<T>::min)() || b == (std::numeric_limits<T>::min)()) {
        return false;
    }
    if(overflowCheck(a, b)) {
        return false;
    }
    a *= b;
    return true;
}

/// Performs a *= b; if it doesn't equal infinity. Returns false otherwise.
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type checked_multiply(T &a, T b) {
    T c = a * b;
    if(std::isinf(c) && !std::isinf(a) && !std::isinf(b)) {
        return false;
    }
    a = c;
    return true;
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
    template <typename T> explicit IsMember(T &&set) : IsMember(std::forward<T>(set), nullptr) {}

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
        desc_function_ = [set]() { return detail::generate_set(detail::smart_deref(set)); };

        // This is the function that validates
        // It stores a copy of the set pointer-like, so shared_ptr will stay alive
        func_ = [set, filter_fn](std::string &input) {
            local_item_t b;
            if(!detail::lexical_cast(input, b)) {
                throw ValidationError(input); // name is added later
            }
            if(filter_fn) {
                b = filter_fn(b);
            }
            auto res = detail::search(set, b, filter_fn);
            if(res.first) {
                // Make sure the version in the input string is identical to the one in the set
                if(filter_fn) {
                    input = detail::to_string(detail::pair_adaptor<element_t>::first(*(res.second)));
                }

                // Return empty error string (success)
                return std::string{};
            }

            // If you reach this point, the result was not found
            std::string out(" not in ");
            out += detail::generate_set(detail::smart_deref(set));
            return out;
        };
    }

    /// You can pass in as many filter functions as you like, they nest (string only currently)
    template <typename T, typename... Args>
    IsMember(T &&set, filter_fn_t filter_fn_1, filter_fn_t filter_fn_2, Args &&... other)
        : IsMember(std::forward<T>(set),
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
    template <typename T> explicit Transformer(T &&mapping) : Transformer(std::forward<T>(mapping), nullptr) {}

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
        desc_function_ = [mapping]() { return detail::generate_map(detail::smart_deref(mapping)); };

        func_ = [mapping, filter_fn](std::string &input) {
            local_item_t b;
            if(!detail::lexical_cast(input, b)) {
                return std::string();
                // there is no possible way we can match anything in the mapping if we can't convert so just return
            }
            if(filter_fn) {
                b = filter_fn(b);
            }
            auto res = detail::search(mapping, b, filter_fn);
            if(res.first) {
                input = detail::to_string(detail::pair_adaptor<element_t>::second(*res.second));
            }
            return std::string{};
        };
    }

    /// You can pass in as many filter functions as you like, they nest
    template <typename T, typename... Args>
    Transformer(T &&mapping, filter_fn_t filter_fn_1, filter_fn_t filter_fn_2, Args &&... other)
        : Transformer(std::forward<T>(mapping),
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
                [](const iteration_type_t &v) { return detail::to_string(detail::pair_adaptor<element_t>::second(v)); },
                ",");
            out.push_back('}');
            return out;
        };

        desc_function_ = tfunc;

        func_ = [mapping, tfunc, filter_fn](std::string &input) {
            local_item_t b;
            bool converted = detail::lexical_cast(input, b);
            if(converted) {
                if(filter_fn) {
                    b = filter_fn(b);
                }
                auto res = detail::search(mapping, b, filter_fn);
                if(res.first) {
                    input = detail::to_string(detail::pair_adaptor<element_t>::second(*res.second));
                    return std::string{};
                }
            }
            for(const auto &v : detail::smart_deref(mapping)) {
                auto output_string = detail::to_string(detail::pair_adaptor<element_t>::second(v));
                if(output_string == input) {
                    return std::string();
                }
            }

            return "Check " + input + " " + tfunc() + " FAILED";
        };
    }

    /// You can pass in as many filter functions as you like, they nest
    template <typename T, typename... Args>
    CheckedTransformer(T &&mapping, filter_fn_t filter_fn_1, filter_fn_t filter_fn_2, Args &&... other)
        : CheckedTransformer(std::forward<T>(mapping),
                             [filter_fn_1, filter_fn_2](std::string a) { return filter_fn_2(filter_fn_1(a)); },
                             other...) {}
};

/// Helper function to allow ignore_case to be passed to IsMember or Transform
inline std::string ignore_case(std::string item) { return detail::to_lower(item); }

/// Helper function to allow ignore_underscore to be passed to IsMember or Transform
inline std::string ignore_underscore(std::string item) { return detail::remove_underscore(item); }

/// Helper function to allow checks to ignore spaces to be passed to IsMember or Transform
inline std::string ignore_space(std::string item) {
    item.erase(std::remove(std::begin(item), std::end(item), ' '), std::end(item));
    item.erase(std::remove(std::begin(item), std::end(item), '\t'), std::end(item));
    return item;
}

/// Multiply a number by a factor using given mapping.
/// Can be used to write transforms for SIZE or DURATION inputs.
///
/// Example:
///   With mapping = `{"b"->1, "kb"->1024, "mb"->1024*1024}`
///   one can recognize inputs like "100", "12kb", "100 MB",
///   that will be automatically transformed to 100, 14448, 104857600.
///
/// Output number type matches the type in the provided mapping.
/// Therefore, if it is required to interpret real inputs like "0.42 s",
/// the mapping should be of a type <string, float> or <string, double>.
class AsNumberWithUnit : public Validator {
  public:
    /// Adjust AsNumberWithUnit behavior.
    /// CASE_SENSITIVE/CASE_INSENSITIVE controls how units are matched.
    /// UNIT_OPTIONAL/UNIT_REQUIRED throws ValidationError
    ///   if UNIT_REQUIRED is set and unit literal is not found.
    enum Options {
        CASE_SENSITIVE = 0,
        CASE_INSENSITIVE = 1,
        UNIT_OPTIONAL = 0,
        UNIT_REQUIRED = 2,
        DEFAULT = CASE_INSENSITIVE | UNIT_OPTIONAL
    };

    template <typename Number>
    explicit AsNumberWithUnit(std::map<std::string, Number> mapping,
                              Options opts = DEFAULT,
                              const std::string &unit_name = "UNIT") {
        description(generate_description<Number>(unit_name, opts));
        validate_mapping(mapping, opts);

        // transform function
        func_ = [mapping, opts](std::string &input) -> std::string {
            Number num;

            detail::rtrim(input);
            if(input.empty()) {
                throw ValidationError("Input is empty");
            }

            // Find split position between number and prefix
            auto unit_begin = input.end();
            while(unit_begin > input.begin() && std::isalpha(*(unit_begin - 1), std::locale())) {
                --unit_begin;
            }

            std::string unit{unit_begin, input.end()};
            input.resize(static_cast<size_t>(std::distance(input.begin(), unit_begin)));
            detail::trim(input);

            if(opts & UNIT_REQUIRED && unit.empty()) {
                throw ValidationError("Missing mandatory unit");
            }
            if(opts & CASE_INSENSITIVE) {
                unit = detail::to_lower(unit);
            }

            bool converted = detail::lexical_cast(input, num);
            if(!converted) {
                throw ValidationError("Value " + input + " could not be converted to " + detail::type_name<Number>());
            }

            if(unit.empty()) {
                // No need to modify input if no unit passed
                return {};
            }

            // find corresponding factor
            auto it = mapping.find(unit);
            if(it == mapping.end()) {
                throw ValidationError(unit +
                                      " unit not recognized. "
                                      "Allowed values: " +
                                      detail::generate_map(mapping, true));
            }

            // perform safe multiplication
            bool ok = detail::checked_multiply(num, it->second);
            if(!ok) {
                throw ValidationError(detail::to_string(num) + " multiplied by " + unit +
                                      " factor would cause number overflow. Use smaller value.");
            }
            input = detail::to_string(num);

            return {};
        };
    }

  private:
    /// Check that mapping contains valid units.
    /// Update mapping for CASE_INSENSITIVE mode.
    template <typename Number> static void validate_mapping(std::map<std::string, Number> &mapping, Options opts) {
        for(auto &kv : mapping) {
            if(kv.first.empty()) {
                throw ValidationError("Unit must not be empty.");
            }
            if(!detail::isalpha(kv.first)) {
                throw ValidationError("Unit must contain only letters.");
            }
        }

        // make all units lowercase if CASE_INSENSITIVE
        if(opts & CASE_INSENSITIVE) {
            std::map<std::string, Number> lower_mapping;
            for(auto &kv : mapping) {
                auto s = detail::to_lower(kv.first);
                if(lower_mapping.count(s)) {
                    throw ValidationError("Several matching lowercase unit representations are found: " + s);
                }
                lower_mapping[detail::to_lower(kv.first)] = kv.second;
            }
            mapping = std::move(lower_mapping);
        }
    }

    /// Generate description like this: NUMBER [UNIT]
    template <typename Number> static std::string generate_description(const std::string &name, Options opts) {
        std::stringstream out;
        out << detail::type_name<Number>() << ' ';
        if(opts & UNIT_REQUIRED) {
            out << name;
        } else {
            out << '[' << name << ']';
        }
        return out.str();
    }
};

/// Converts a human-readable size string (with unit literal) to uin64_t size.
/// Example:
///   "100" => 100
///   "1 b" => 100
///   "10Kb" => 10240 // you can configure this to be interpreted as kilobyte (*1000) or kibibyte (*1024)
///   "10 KB" => 10240
///   "10 kb" => 10240
///   "10 kib" => 10240 // *i, *ib are always interpreted as *bibyte (*1024)
///   "10kb" => 10240
///   "2 MB" => 2097152
///   "2 EiB" => 2^61 // Units up to exibyte are supported
class AsSizeValue : public AsNumberWithUnit {
  public:
    using result_t = uint64_t;

    /// If kb_is_1000 is true,
    /// interpret 'kb', 'k' as 1000 and 'kib', 'ki' as 1024
    /// (same applies to higher order units as well).
    /// Otherwise, interpret all literals as factors of 1024.
    /// The first option is formally correct, but
    /// the second interpretation is more wide-spread
    /// (see https://en.wikipedia.org/wiki/Binary_prefix).
    explicit AsSizeValue(bool kb_is_1000) : AsNumberWithUnit(get_mapping(kb_is_1000)) {
        if(kb_is_1000) {
            description("SIZE [b, kb(=1000b), kib(=1024b), ...]");
        } else {
            description("SIZE [b, kb(=1024b), ...]");
        }
    }

  private:
    /// Get <size unit, factor> mapping
    static std::map<std::string, result_t> init_mapping(bool kb_is_1000) {
        std::map<std::string, result_t> m;
        result_t k_factor = kb_is_1000 ? 1000 : 1024;
        result_t ki_factor = 1024;
        result_t k = 1;
        result_t ki = 1;
        m["b"] = 1;
        for(std::string p : {"k", "m", "g", "t", "p", "e"}) {
            k *= k_factor;
            ki *= ki_factor;
            m[p] = k;
            m[p + "b"] = k;
            m[p + "i"] = ki;
            m[p + "ib"] = ki;
        }
        return m;
    }

    /// Cache calculated mapping
    static std::map<std::string, result_t> get_mapping(bool kb_is_1000) {
        if(kb_is_1000) {
            static auto m = init_mapping(true);
            return m;
        } else {
            static auto m = init_mapping(false);
            return m;
        }
    }
};

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
