#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include "StringTools.hpp"
#include <exception>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace CLI {

// Type tools

// Utilities for type enabling
namespace detail {
// Based generally on https://rmf.io/cxx11/almost-static-if
/// Simple empty scoped class
enum class enabler {};

/// An instance to use in EnableIf
constexpr enabler dummy = {};
} // namespace detail

/// A copy of enable_if_t from C++14, compatible with C++11.
///
/// We could check to see if C++14 is being used, but it does not hurt to redefine this
/// (even Google does this: https://github.com/google/skia/blob/master/include/private/SkTLogic.h)
/// It is not in the std namespace anyway, so no harm done.
template <bool B, class T = void> using enable_if_t = typename std::enable_if<B, T>::type;

/// A copy of std::void_t from C++17 (helper for C++11 and C++14)
template <typename... Ts> struct make_void { using type = void; };

/// A copy of std::void_t from C++17 - same reasoning as enable_if_t, it does not hurt to redefine
template <typename... Ts> using void_t = typename make_void<Ts...>::type;

/// A copy of std::conditional_t from C++14 - same reasoning as enable_if_t, it does not hurt to redefine
template <bool B, class T, class F> using conditional_t = typename std::conditional<B, T, F>::type;

/// Check to see if something is a vector (fail check by default)
template <typename T> struct is_vector : std::false_type {};

/// Check to see if something is a vector (true if actually a vector)
template <class T, class A> struct is_vector<std::vector<T, A>> : std::true_type {};

/// Check to see if something is bool (fail check by default)
template <typename T> struct is_bool : std::false_type {};

/// Check to see if something is bool (true if actually a bool)
template <> struct is_bool<bool> : std::true_type {};

/// Check to see if something is a shared pointer
template <typename T> struct is_shared_ptr : std::false_type {};

/// Check to see if something is a shared pointer (True if really a shared pointer)
template <typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

/// Check to see if something is a shared pointer (True if really a shared pointer)
template <typename T> struct is_shared_ptr<const std::shared_ptr<T>> : std::true_type {};

/// Check to see if something is copyable pointer
template <typename T> struct is_copyable_ptr {
    static bool const value = is_shared_ptr<T>::value || std::is_pointer<T>::value;
};

/// This can be specialized to override the type deduction for IsMember.
template <typename T> struct IsMemberType { using type = T; };

/// The main custom type needed here is const char * should be a string.
template <> struct IsMemberType<const char *> { using type = std::string; };

namespace detail {

// These are utilities for IsMember

/// Handy helper to access the element_type generically. This is not part of is_copyable_ptr because it requires that
/// pointer_traits<T> be valid.
template <typename T> struct element_type {
    using type =
        typename std::conditional<is_copyable_ptr<T>::value, typename std::pointer_traits<T>::element_type, T>::type;
};

/// Combination of the element type and value type - remove pointer (including smart pointers) and get the value_type of
/// the container
template <typename T> struct element_value_type { using type = typename element_type<T>::type::value_type; };

/// Adaptor for set-like structure: This just wraps a normal container in a few utilities that do almost nothing.
template <typename T, typename _ = void> struct pair_adaptor : std::false_type {
    using value_type = typename T::value_type;
    using first_type = typename std::remove_const<value_type>::type;
    using second_type = typename std::remove_const<value_type>::type;

    /// Get the first value (really just the underlying value)
    template <typename Q> static auto first(Q &&pair_value) -> decltype(std::forward<Q>(pair_value)) {
        return std::forward<Q>(pair_value);
    }
    /// Get the second value (really just the underlying value)
    template <typename Q> static auto second(Q &&pair_value) -> decltype(std::forward<Q>(pair_value)) {
        return std::forward<Q>(pair_value);
    }
};

/// Adaptor for map-like structure (true version, must have key_type and mapped_type).
/// This wraps a mapped container in a few utilities access it in a general way.
template <typename T>
struct pair_adaptor<
    T,
    conditional_t<false, void_t<typename T::value_type::first_type, typename T::value_type::second_type>, void>>
    : std::true_type {
    using value_type = typename T::value_type;
    using first_type = typename std::remove_const<typename value_type::first_type>::type;
    using second_type = typename std::remove_const<typename value_type::second_type>::type;

    /// Get the first value (really just the underlying value)
    template <typename Q> static auto first(Q &&pair_value) -> decltype(std::get<0>(std::forward<Q>(pair_value))) {
        return std::get<0>(std::forward<Q>(pair_value));
    }
    /// Get the second value (really just the underlying value)
    template <typename Q> static auto second(Q &&pair_value) -> decltype(std::get<1>(std::forward<Q>(pair_value))) {
        return std::get<1>(std::forward<Q>(pair_value));
    }
};

// Check for streamability
// Based on https://stackoverflow.com/questions/22758291/how-can-i-detect-if-a-type-can-be-streamed-to-an-stdostream

template <typename S, typename T> class is_streamable {
    template <typename SS, typename TT>
    static auto test(int) -> decltype(std::declval<SS &>() << std::declval<TT>(), std::true_type());

    template <typename, typename> static auto test(...) -> std::false_type;

  public:
    static const bool value = decltype(test<S, T>(0))::value;
};

/// Convert an object to a string (directly forward if this can become a string)
template <typename T, enable_if_t<std::is_constructible<std::string, T>::value, detail::enabler> = detail::dummy>
auto to_string(T &&value) -> decltype(std::forward<T>(value)) {
    return std::forward<T>(value);
}

/// Convert an object to a string (streaming must be supported for that type)
template <typename T,
          enable_if_t<!std::is_constructible<std::string, T>::value && is_streamable<std::stringstream, T>::value,
                      detail::enabler> = detail::dummy>
std::string to_string(T &&value) {
    std::stringstream stream;
    stream << value;
    return stream.str();
}

/// If conversion is not supported, return an empty string (streaming is not supported for that type)
template <typename T,
          enable_if_t<!std::is_constructible<std::string, T>::value && !is_streamable<std::stringstream, T>::value,
                      detail::enabler> = detail::dummy>
std::string to_string(T &&) {
    return std::string{};
}

// Type name print

/// Was going to be based on
///  http://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
/// But this is cleaner and works better in this case

template <typename T,
          enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "INT";
}

template <typename T,
          enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "UINT";
}

template <typename T, enable_if_t<std::is_floating_point<T>::value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "FLOAT";
}

/// This one should not be used, since vector types print the internal type
template <typename T, enable_if_t<is_vector<T>::value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "VECTOR";
}
/// Print name for enumeration types
template <typename T, enable_if_t<std::is_enum<T>::value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "ENUM";
}

/// Print for all other types
template <typename T,
          enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value && !is_vector<T>::value &&
                          !std::is_enum<T>::value,
                      detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "TEXT";
}

// Lexical cast

/// Convert a flag into an integer value  typically binary flags
inline int64_t to_flag_value(std::string val) {
    static const std::string trueString("true");
    static const std::string falseString("false");
    if(val == trueString) {
        return 1;
    }
    if(val == falseString) {
        return -1;
    }
    val = detail::to_lower(val);
    int64_t ret;
    if(val.size() == 1) {
        switch(val[0]) {
        case '0':
        case 'f':
        case 'n':
        case '-':
            ret = -1;
            break;
        case '1':
        case 't':
        case 'y':
        case '+':
            ret = 1;
            break;
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            ret = val[0] - '0';
            break;
        default:
            throw std::invalid_argument("unrecognized character");
        }
        return ret;
    }
    if(val == trueString || val == "on" || val == "yes" || val == "enable") {
        ret = 1;
    } else if(val == falseString || val == "off" || val == "no" || val == "disable") {
        ret = -1;
    } else {
        ret = std::stoll(val);
    }
    return ret;
}

/// Signed integers
template <
    typename T,
    enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value && !is_bool<T>::value && !std::is_enum<T>::value,
                detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {
    try {
        size_t n = 0;
        long long output_ll = std::stoll(input, &n, 0);
        output = static_cast<T>(output_ll);
        return n == input.size() && static_cast<long long>(output) == output_ll;
    } catch(const std::invalid_argument &) {
        return false;
    } catch(const std::out_of_range &) {
        return false;
    }
}

/// Unsigned integers
template <typename T,
          enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value && !is_bool<T>::value, detail::enabler> =
              detail::dummy>
bool lexical_cast(std::string input, T &output) {
    if(!input.empty() && input.front() == '-')
        return false; // std::stoull happily converts negative values to junk without any errors.

    try {
        size_t n = 0;
        unsigned long long output_ll = std::stoull(input, &n, 0);
        output = static_cast<T>(output_ll);
        return n == input.size() && static_cast<unsigned long long>(output) == output_ll;
    } catch(const std::invalid_argument &) {
        return false;
    } catch(const std::out_of_range &) {
        return false;
    }
}

/// Boolean values
template <typename T, enable_if_t<is_bool<T>::value, detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {
    try {
        auto out = to_flag_value(input);
        output = (out > 0);
        return true;
    } catch(const std::invalid_argument &) {
        return false;
    }
}

/// Floats
template <typename T, enable_if_t<std::is_floating_point<T>::value, detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {
    try {
        size_t n = 0;
        output = static_cast<T>(std::stold(input, &n));
        return n == input.size();
    } catch(const std::invalid_argument &) {
        return false;
    } catch(const std::out_of_range &) {
        return false;
    }
}

/// String and similar
template <typename T,
          enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value &&
                          std::is_assignable<T &, std::string>::value,
                      detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {
    output = input;
    return true;
}

/// Enumerations
template <typename T, enable_if_t<std::is_enum<T>::value, detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {
    typename std::underlying_type<T>::type val;
    bool retval = detail::lexical_cast(input, val);
    if(!retval) {
        return false;
    }
    output = static_cast<T>(val);
    return true;
}

/// Non-string parsable
template <typename T,
          enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value &&
                          !std::is_assignable<T &, std::string>::value && !std::is_enum<T>::value,
                      detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {
    std::istringstream is;

    is.str(input);
    is >> output;
    return !is.fail() && !is.rdbuf()->in_avail();
}

/// Sum a vector of flag representations
/// The flag vector produces a series of strings in a vector,  simple true is represented by a "1",  simple false is by
/// "-1" an if numbers are passed by some fashion they are captured as well so the function just checks for the most
/// common true and false strings then uses stoll to convert the rest for summing
template <typename T,
          enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, detail::enabler> = detail::dummy>
void sum_flag_vector(const std::vector<std::string> &flags, T &output) {
    int64_t count{0};
    for(auto &flag : flags) {
        count += detail::to_flag_value(flag);
    }
    output = (count > 0) ? static_cast<T>(count) : T{0};
}

/// Sum a vector of flag representations
/// The flag vector produces a series of strings in a vector,  simple true is represented by a "1",  simple false is by
/// "-1" an if numbers are passed by some fashion they are captured as well so the function just checks for the most
/// common true and false strings then uses stoll to convert the rest for summing
template <typename T,
          enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, detail::enabler> = detail::dummy>
void sum_flag_vector(const std::vector<std::string> &flags, T &output) {
    int64_t count{0};
    for(auto &flag : flags) {
        count += detail::to_flag_value(flag);
    }
    output = static_cast<T>(count);
}

} // namespace detail
} // namespace CLI
