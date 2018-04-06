#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <exception>
#include <string>
#include <type_traits>
#include <vector>

namespace CLI {

// Type tools

// We could check to see if C++14 is being used, but it does not hurt to redefine this
// (even Google does this: https://github.com/google/skia/blob/master/include/private/SkTLogic.h)
// It is not in the std namespace anyway, so no harm done.

template <bool B, class T = void> using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T> struct is_vector { static const bool value = false; };

template <class T, class A> struct is_vector<std::vector<T, A>> { static bool const value = true; };

template <typename T> struct is_bool { static const bool value = false; };

template <> struct is_bool<bool> { static bool const value = true; };

namespace detail {
// Based generally on https://rmf.io/cxx11/almost-static-if
/// Simple empty scoped class
enum class enabler {};

/// An instance to use in EnableIf
constexpr enabler dummy = {};

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

template <typename T,
          enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value && !is_vector<T>::value,
                      detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "TEXT";
}

// Lexical cast

/// Signed integers / enums
template <typename T,
          enable_if_t<(std::is_integral<T>::value && std::is_signed<T>::value), detail::enabler> = detail::dummy>
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
          enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, detail::enabler> = detail::dummy>
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

/// Non-string parsable
template <typename T,
          enable_if_t<!std::is_floating_point<T>::value && !std::is_integral<T>::value &&
                          !std::is_assignable<T &, std::string>::value,
                      detail::enabler> = detail::dummy>
bool lexical_cast(std::string input, T &output) {

// On GCC 4.7, thread_local is not available, so this optimization
// is turned off (avoiding multiple initialisations on multiple usages)
#if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER) && __GNUC__ == 4 && (__GNUC_MINOR__ < 8)
    std::istringstream is;
#else
    static thread_local std::istringstream is;
    is.clear();
#endif

    is.str(input);
    is >> output;
    return !is.fail() && !is.rdbuf()->in_avail();
}

} // namespace detail
} // namespace CLI
