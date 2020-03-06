// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "StringTools.hpp"
#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
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
}  // namespace detail

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

/// Check to see if something is a vector (true if actually a const vector)
template <class T, class A> struct is_vector<const std::vector<T, A>> : std::true_type {};

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

// These are utilities for IsMember and other transforming objects

/// Handy helper to access the element_type generically. This is not part of is_copyable_ptr because it requires that
/// pointer_traits<T> be valid.

/// not a pointer
template <typename T, typename Enable = void> struct element_type { using type = T; };

template <typename T> struct element_type<T, typename std::enable_if<is_copyable_ptr<T>::value>::type> {
    using type = typename std::pointer_traits<T>::element_type;
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

// Warning is suppressed due to "bug" in gcc<5.0 and gcc 7.0 with c++17 enabled that generates a Wnarrowing warning
// in the unevaluated context even if the function that was using this wasn't used.  The standard says narrowing in
// brace initialization shouldn't be allowed but for backwards compatibility gcc allows it in some contexts.  It is a
// little fuzzy what happens in template constructs and I think that was something GCC took a little while to work out.
// But regardless some versions of gcc generate a warning when they shouldn't from the following code so that should be
// suppressed
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
#endif
// check for constructibility from a specific type and copy assignable used in the parse detection
template <typename T, typename C> class is_direct_constructible {
    template <typename TT, typename CC>
    static auto test(int, std::true_type) -> decltype(
// NVCC warns about narrowing conversions here
#ifdef __CUDACC__
#pragma diag_suppress 2361
#endif
        TT { std::declval<CC>() }
#ifdef __CUDACC__
#pragma diag_default 2361
#endif
        ,
        std::is_move_assignable<TT>());

    template <typename TT, typename CC> static auto test(int, std::false_type) -> std::false_type;

    template <typename, typename> static auto test(...) -> std::false_type;

  public:
    static constexpr bool value = decltype(test<T, C>(0, typename std::is_constructible<T, C>::type()))::value;
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

// Check for output streamability
// Based on https://stackoverflow.com/questions/22758291/how-can-i-detect-if-a-type-can-be-streamed-to-an-stdostream

template <typename T, typename S = std::ostringstream> class is_ostreamable {
    template <typename TT, typename SS>
    static auto test(int) -> decltype(std::declval<SS &>() << std::declval<TT>(), std::true_type());

    template <typename, typename> static auto test(...) -> std::false_type;

  public:
    static constexpr bool value = decltype(test<T, S>(0))::value;
};

/// Check for input streamability
template <typename T, typename S = std::istringstream> class is_istreamable {
    template <typename TT, typename SS>
    static auto test(int) -> decltype(std::declval<SS &>() >> std::declval<TT &>(), std::true_type());

    template <typename, typename> static auto test(...) -> std::false_type;

  public:
    static constexpr bool value = decltype(test<T, S>(0))::value;
};

/// Templated operation to get a value from a stream
template <typename T, enable_if_t<is_istreamable<T>::value, detail::enabler> = detail::dummy>
bool from_stream(const std::string &istring, T &obj) {
    std::istringstream is;
    is.str(istring);
    is >> obj;
    return !is.fail() && !is.rdbuf()->in_avail();
}

template <typename T, enable_if_t<!is_istreamable<T>::value, detail::enabler> = detail::dummy>
bool from_stream(const std::string & /*istring*/, T & /*obj*/) {
    return false;
}

// Check for tuple like types, as in classes with a tuple_size type trait
template <typename S> class is_tuple_like {
    template <typename SS>
    // static auto test(int)
    //     -> decltype(std::conditional<(std::tuple_size<SS>::value > 0), std::true_type, std::false_type>::type());
    static auto test(int) -> decltype(std::tuple_size<SS>::value, std::true_type{});
    template <typename> static auto test(...) -> std::false_type;

  public:
    static constexpr bool value = decltype(test<S>(0))::value;
};

/// Convert an object to a string (directly forward if this can become a string)
template <typename T, enable_if_t<std::is_convertible<T, std::string>::value, detail::enabler> = detail::dummy>
auto to_string(T &&value) -> decltype(std::forward<T>(value)) {
    return std::forward<T>(value);
}

/// Construct a string from the object
template <typename T,
          enable_if_t<std::is_constructible<std::string, T>::value && !std::is_convertible<T, std::string>::value,
                      detail::enabler> = detail::dummy>
std::string to_string(const T &value) {
    return std::string(value);
}

/// Convert an object to a string (streaming must be supported for that type)
template <typename T,
          enable_if_t<!std::is_convertible<std::string, T>::value && !std::is_constructible<std::string, T>::value &&
                          is_ostreamable<T>::value,
                      detail::enabler> = detail::dummy>
std::string to_string(T &&value) {
    std::stringstream stream;
    stream << value;
    return stream.str();
}

/// If conversion is not supported, return an empty string (streaming is not supported for that type)
template <typename T,
          enable_if_t<!std::is_constructible<std::string, T>::value && !is_ostreamable<T>::value &&
                          !is_vector<typename std::remove_reference<typename std::remove_const<T>::type>::type>::value,
                      detail::enabler> = detail::dummy>
std::string to_string(T &&) {
    return std::string{};
}

/// convert a vector to a string
template <typename T,
          enable_if_t<!std::is_constructible<std::string, T>::value && !is_ostreamable<T>::value &&
                          is_vector<typename std::remove_reference<typename std::remove_const<T>::type>::type>::value,
                      detail::enabler> = detail::dummy>
std::string to_string(T &&variable) {
    std::vector<std::string> defaults;
    defaults.reserve(variable.size());
    auto cval = variable.begin();
    auto end = variable.end();
    while(cval != end) {
        defaults.emplace_back(CLI::detail::to_string(*cval));
        ++cval;
    }
    return std::string("[" + detail::join(defaults) + "]");
}

/// special template overload
template <typename T1,
          typename T2,
          typename T,
          enable_if_t<std::is_same<T1, T2>::value, detail::enabler> = detail::dummy>
auto checked_to_string(T &&value) -> decltype(to_string(std::forward<T>(value))) {
    return to_string(std::forward<T>(value));
}

/// special template overload
template <typename T1,
          typename T2,
          typename T,
          enable_if_t<!std::is_same<T1, T2>::value, detail::enabler> = detail::dummy>
std::string checked_to_string(T &&) {
    return std::string{};
}
/// get a string as a convertible value for arithmetic types
template <typename T, enable_if_t<std::is_arithmetic<T>::value, detail::enabler> = detail::dummy>
std::string value_string(const T &value) {
    return std::to_string(value);
}
/// get a string as a convertible value for enumerations
template <typename T, enable_if_t<std::is_enum<T>::value, detail::enabler> = detail::dummy>
std::string value_string(const T &value) {
    return std::to_string(static_cast<typename std::underlying_type<T>::type>(value));
}
/// for other types just use the regular to_string function
template <typename T,
          enable_if_t<!std::is_enum<T>::value && !std::is_arithmetic<T>::value, detail::enabler> = detail::dummy>
auto value_string(const T &value) -> decltype(to_string(value)) {
    return to_string(value);
}

/// This will only trigger for actual void type
template <typename T, typename Enable = void> struct type_count { static const int value{0}; };

/// Set of overloads to get the type size of an object
template <typename T> struct type_count<T, typename std::enable_if<is_tuple_like<T>::value>::type> {
    static constexpr int value{std::tuple_size<T>::value};
};
/// Type size for regular object types that do not look like a tuple
template <typename T>
struct type_count<
    T,
    typename std::enable_if<!is_vector<T>::value && !is_tuple_like<T>::value && !std::is_void<T>::value>::type> {
    static constexpr int value{1};
};

/// Type size of types that look like a vector
template <typename T> struct type_count<T, typename std::enable_if<is_vector<T>::value>::type> {
    static constexpr int value{is_vector<typename T::value_type>::value ? expected_max_vector_size
                                                                        : type_count<typename T::value_type>::value};
};

/// This will only trigger for actual void type
template <typename T, typename Enable = void> struct expected_count { static const int value{0}; };

/// For most types the number of expected items is 1
template <typename T>
struct expected_count<T, typename std::enable_if<!is_vector<T>::value && !std::is_void<T>::value>::type> {
    static constexpr int value{1};
};
/// number of expected items in a vector
template <typename T> struct expected_count<T, typename std::enable_if<is_vector<T>::value>::type> {
    static constexpr int value{expected_max_vector_size};
};

// Enumeration of the different supported categorizations of objects
enum class object_category : int {
    integral_value = 2,
    unsigned_integral = 4,
    enumeration = 6,
    boolean_value = 8,
    floating_point = 10,
    number_constructible = 12,
    double_constructible = 14,
    integer_constructible = 16,
    vector_value = 30,
    tuple_value = 35,
    // string assignable or greater used in a condition so anything string like must come last
    string_assignable = 50,
    string_constructible = 60,
    other = 200,

};

/// some type that is not otherwise recognized
template <typename T, typename Enable = void> struct classify_object {
    static constexpr object_category value{object_category::other};
};

/// Set of overloads to classify an object according to type
template <typename T>
struct classify_object<T,
                       typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value &&
                                               !is_bool<T>::value && !std::is_enum<T>::value>::type> {
    static constexpr object_category value{object_category::integral_value};
};

/// Unsigned integers
template <typename T>
struct classify_object<
    T,
    typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value && !is_bool<T>::value>::type> {
    static constexpr object_category value{object_category::unsigned_integral};
};

/// Boolean values
template <typename T> struct classify_object<T, typename std::enable_if<is_bool<T>::value>::type> {
    static constexpr object_category value{object_category::boolean_value};
};

/// Floats
template <typename T> struct classify_object<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    static constexpr object_category value{object_category::floating_point};
};

/// String and similar direct assignment
template <typename T>
struct classify_object<
    T,
    typename std::enable_if<!std::is_floating_point<T>::value && !std::is_integral<T>::value &&
                            std::is_assignable<T &, std::string>::value && !is_vector<T>::value>::type> {
    static constexpr object_category value{object_category::string_assignable};
};

/// String and similar constructible and copy assignment
template <typename T>
struct classify_object<
    T,
    typename std::enable_if<!std::is_floating_point<T>::value && !std::is_integral<T>::value &&
                            !std::is_assignable<T &, std::string>::value &&
                            std::is_constructible<T, std::string>::value && !is_vector<T>::value>::type> {
    static constexpr object_category value{object_category::string_constructible};
};

/// Enumerations
template <typename T> struct classify_object<T, typename std::enable_if<std::is_enum<T>::value>::type> {
    static constexpr object_category value{object_category::enumeration};
};

/// Handy helper to contain a bunch of checks that rule out many common types (integers, string like, floating point,
/// vectors, and enumerations
template <typename T> struct uncommon_type {
    using type = typename std::conditional<!std::is_floating_point<T>::value && !std::is_integral<T>::value &&
                                               !std::is_assignable<T &, std::string>::value &&
                                               !std::is_constructible<T, std::string>::value && !is_vector<T>::value &&
                                               !std::is_enum<T>::value,
                                           std::true_type,
                                           std::false_type>::type;
    static constexpr bool value = type::value;
};

/// Assignable from double or int
template <typename T>
struct classify_object<T,
                       typename std::enable_if<uncommon_type<T>::value && type_count<T>::value == 1 &&
                                               is_direct_constructible<T, double>::value &&
                                               is_direct_constructible<T, int>::value>::type> {
    static constexpr object_category value{object_category::number_constructible};
};

/// Assignable from int
template <typename T>
struct classify_object<T,
                       typename std::enable_if<uncommon_type<T>::value && type_count<T>::value == 1 &&
                                               !is_direct_constructible<T, double>::value &&
                                               is_direct_constructible<T, int>::value>::type> {
    static constexpr object_category value{object_category::integer_constructible};
};

/// Assignable from double
template <typename T>
struct classify_object<T,
                       typename std::enable_if<uncommon_type<T>::value && type_count<T>::value == 1 &&
                                               is_direct_constructible<T, double>::value &&
                                               !is_direct_constructible<T, int>::value>::type> {
    static constexpr object_category value{object_category::double_constructible};
};

/// Tuple type
template <typename T>
struct classify_object<T,
                       typename std::enable_if<(type_count<T>::value >= 2 && !is_vector<T>::value) ||
                                               (is_tuple_like<T>::value && uncommon_type<T>::value &&
                                                !is_direct_constructible<T, double>::value &&
                                                !is_direct_constructible<T, int>::value)>::type> {
    static constexpr object_category value{object_category::tuple_value};
};

/// Vector type
template <typename T> struct classify_object<T, typename std::enable_if<is_vector<T>::value>::type> {
    static constexpr object_category value{object_category::vector_value};
};

// Type name print

/// Was going to be based on
///  http://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
/// But this is cleaner and works better in this case

template <typename T,
          enable_if_t<classify_object<T>::value == object_category::integral_value ||
                          classify_object<T>::value == object_category::integer_constructible,
                      detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "INT";
}

template <typename T,
          enable_if_t<classify_object<T>::value == object_category::unsigned_integral, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "UINT";
}

template <typename T,
          enable_if_t<classify_object<T>::value == object_category::floating_point ||
                          classify_object<T>::value == object_category::number_constructible ||
                          classify_object<T>::value == object_category::double_constructible,
                      detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "FLOAT";
}

/// Print name for enumeration types
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::enumeration, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "ENUM";
}

/// Print name for enumeration types
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::boolean_value, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "BOOLEAN";
}

/// Print for all other types
template <typename T,
          enable_if_t<classify_object<T>::value >= object_category::string_assignable, detail::enabler> = detail::dummy>
constexpr const char *type_name() {
    return "TEXT";
}

/// Print name for single element tuple types
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::tuple_value && type_count<T>::value == 1,
                      detail::enabler> = detail::dummy>
inline std::string type_name() {
    return type_name<typename std::tuple_element<0, T>::type>();
}

/// Empty string if the index > tuple size
template <typename T, std::size_t I>
inline typename std::enable_if<I == type_count<T>::value, std::string>::type tuple_name() {
    return std::string{};
}

/// Recursively generate the tuple type name
template <typename T, std::size_t I>
    inline typename std::enable_if < I<type_count<T>::value, std::string>::type tuple_name() {
    std::string str = std::string(type_name<typename std::tuple_element<I, T>::type>()) + ',' + tuple_name<T, I + 1>();
    if(str.back() == ',')
        str.pop_back();
    return str;
}

/// Print type name for tuples with 2 or more elements
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::tuple_value && type_count<T>::value >= 2,
                      detail::enabler> = detail::dummy>
std::string type_name() {
    auto tname = std::string(1, '[') + tuple_name<T, 0>();
    tname.push_back(']');
    return tname;
}

/// This one should not be used normally, since vector types print the internal type
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::vector_value, detail::enabler> = detail::dummy>
inline std::string type_name() {
    return type_name<typename T::value_type>();
}

// Lexical cast

/// Convert a flag into an integer value  typically binary flags
inline std::int64_t to_flag_value(std::string val) {
    static const std::string trueString("true");
    static const std::string falseString("false");
    if(val == trueString) {
        return 1;
    }
    if(val == falseString) {
        return -1;
    }
    val = detail::to_lower(val);
    std::int64_t ret;
    if(val.size() == 1) {
        if(val[0] >= '1' && val[0] <= '9') {
            return (static_cast<std::int64_t>(val[0]) - '0');
        }
        switch(val[0]) {
        case '0':
        case 'f':
        case 'n':
        case '-':
            ret = -1;
            break;
        case 't':
        case 'y':
        case '+':
            ret = 1;
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
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::integral_value, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    try {
        std::size_t n = 0;
        std::int64_t output_ll = std::stoll(input, &n, 0);
        output = static_cast<T>(output_ll);
        return n == input.size() && static_cast<std::int64_t>(output) == output_ll;
    } catch(const std::invalid_argument &) {
        return false;
    } catch(const std::out_of_range &) {
        return false;
    }
}

/// Unsigned integers
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::unsigned_integral, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    if(!input.empty() && input.front() == '-')
        return false;  // std::stoull happily converts negative values to junk without any errors.

    try {
        std::size_t n = 0;
        std::uint64_t output_ll = std::stoull(input, &n, 0);
        output = static_cast<T>(output_ll);
        return n == input.size() && static_cast<std::uint64_t>(output) == output_ll;
    } catch(const std::invalid_argument &) {
        return false;
    } catch(const std::out_of_range &) {
        return false;
    }
}

/// Boolean values
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::boolean_value, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    try {
        auto out = to_flag_value(input);
        output = (out > 0);
        return true;
    } catch(const std::invalid_argument &) {
        return false;
    } catch(const std::out_of_range &) {
        // if the number is out of the range of a 64 bit value then it is still a number and for this purpose is still
        // valid all we care about the sign
        output = (input[0] != '-');
        return true;
    }
}

/// Floats
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::floating_point, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    try {
        std::size_t n = 0;
        output = static_cast<T>(std::stold(input, &n));
        return n == input.size();
    } catch(const std::invalid_argument &) {
        return false;
    } catch(const std::out_of_range &) {
        return false;
    }
}

/// String and similar direct assignment
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::string_assignable, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    output = input;
    return true;
}

/// String and similar constructible and copy assignment
template <
    typename T,
    enable_if_t<classify_object<T>::value == object_category::string_constructible, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    output = T(input);
    return true;
}

/// Enumerations
template <typename T,
          enable_if_t<classify_object<T>::value == object_category::enumeration, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    typename std::underlying_type<T>::type val;
    bool retval = detail::lexical_cast(input, val);
    if(!retval) {
        return false;
    }
    output = static_cast<T>(val);
    return true;
}

/// Assignable from double or int
template <
    typename T,
    enable_if_t<classify_object<T>::value == object_category::number_constructible, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    int val;
    if(lexical_cast(input, val)) {
        output = T(val);
        return true;
    } else {
        double dval;
        if(lexical_cast(input, dval)) {
            output = T{dval};
            return true;
        }
    }
    return from_stream(input, output);
}

/// Assignable from int
template <
    typename T,
    enable_if_t<classify_object<T>::value == object_category::integer_constructible, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    int val;
    if(lexical_cast(input, val)) {
        output = T(val);
        return true;
    }
    return from_stream(input, output);
}

/// Assignable from double
template <
    typename T,
    enable_if_t<classify_object<T>::value == object_category::double_constructible, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    double val;
    if(lexical_cast(input, val)) {
        output = T{val};
        return true;
    }
    return from_stream(input, output);
}

/// Non-string parsable by a stream
template <typename T, enable_if_t<classify_object<T>::value == object_category::other, detail::enabler> = detail::dummy>
bool lexical_cast(const std::string &input, T &output) {
    static_assert(is_istreamable<T>::value,
                  "option object type must have a lexical cast overload or streaming input operator(>>) defined, if it "
                  "is convertible from another type use the add_option<T, XC>(...) with XC being the known type");
    return from_stream(input, output);
}

/// Assign a value through lexical cast operations
template <
    typename T,
    typename XC,
    enable_if_t<std::is_same<T, XC>::value && (classify_object<T>::value == object_category::string_assignable ||
                                               classify_object<T>::value == object_category::string_constructible),
                detail::enabler> = detail::dummy>
bool lexical_assign(const std::string &input, T &output) {
    return lexical_cast(input, output);
}

/// Assign a value through lexical cast operations
template <typename T,
          typename XC,
          enable_if_t<std::is_same<T, XC>::value && classify_object<T>::value != object_category::string_assignable &&
                          classify_object<T>::value != object_category::string_constructible,
                      detail::enabler> = detail::dummy>
bool lexical_assign(const std::string &input, T &output) {
    if(input.empty()) {
        output = T{};
        return true;
    }
    return lexical_cast(input, output);
}

/// Assign a value converted from a string in lexical cast to the output value directly
template <
    typename T,
    typename XC,
    enable_if_t<!std::is_same<T, XC>::value && std::is_assignable<T &, XC &>::value, detail::enabler> = detail::dummy>
bool lexical_assign(const std::string &input, T &output) {
    XC val{};
    bool parse_result = (!input.empty()) ? lexical_cast<XC>(input, val) : true;
    if(parse_result) {
        output = val;
    }
    return parse_result;
}

/// Assign a value from a lexical cast through constructing a value and move assigning it
template <typename T,
          typename XC,
          enable_if_t<!std::is_same<T, XC>::value && !std::is_assignable<T &, XC &>::value &&
                          std::is_move_assignable<T>::value,
                      detail::enabler> = detail::dummy>
bool lexical_assign(const std::string &input, T &output) {
    XC val{};
    bool parse_result = input.empty() ? true : lexical_cast<XC>(input, val);
    if(parse_result) {
        output = T(val);  // use () form of constructor to allow some implicit conversions
    }
    return parse_result;
}
/// Lexical conversion if there is only one element
template <
    typename T,
    typename XC,
    enable_if_t<!is_tuple_like<T>::value && !is_tuple_like<XC>::value && !is_vector<T>::value && !is_vector<XC>::value,
                detail::enabler> = detail::dummy>
bool lexical_conversion(const std::vector<std ::string> &strings, T &output) {
    return lexical_assign<T, XC>(strings[0], output);
}

/// Lexical conversion if there is only one element but the conversion type is for two call a two element constructor
template <typename T,
          typename XC,
          enable_if_t<type_count<T>::value == 1 && type_count<XC>::value == 2, detail::enabler> = detail::dummy>
bool lexical_conversion(const std::vector<std ::string> &strings, T &output) {
    typename std::tuple_element<0, XC>::type v1;
    typename std::tuple_element<1, XC>::type v2;
    bool retval = lexical_assign<decltype(v1), decltype(v1)>(strings[0], v1);
    if(strings.size() > 1) {
        retval = retval && lexical_assign<decltype(v2), decltype(v2)>(strings[1], v2);
    }
    if(retval) {
        output = T{v1, v2};
    }
    return retval;
}

/// Lexical conversion of a vector types
template <class T,
          class XC,
          enable_if_t<expected_count<T>::value == expected_max_vector_size &&
                          expected_count<XC>::value == expected_max_vector_size && type_count<XC>::value == 1,
                      detail::enabler> = detail::dummy>
bool lexical_conversion(const std::vector<std ::string> &strings, T &output) {
    output.clear();
    output.reserve(strings.size());
    for(const auto &elem : strings) {

        output.emplace_back();
        bool retval = lexical_assign<typename T::value_type, typename XC::value_type>(elem, output.back());
        if(!retval) {
            return false;
        }
    }
    return (!output.empty());
}

/// Lexical conversion of a vector types with type size of two
template <class T,
          class XC,
          enable_if_t<expected_count<T>::value == expected_max_vector_size &&
                          expected_count<XC>::value == expected_max_vector_size && type_count<XC>::value == 2,
                      detail::enabler> = detail::dummy>
bool lexical_conversion(const std::vector<std ::string> &strings, T &output) {
    output.clear();
    for(std::size_t ii = 0; ii < strings.size(); ii += 2) {

        typename std::tuple_element<0, typename XC::value_type>::type v1;
        typename std::tuple_element<1, typename XC::value_type>::type v2;
        bool retval = lexical_assign<decltype(v1), decltype(v1)>(strings[ii], v1);
        if(strings.size() > ii + 1) {
            retval = retval && lexical_assign<decltype(v2), decltype(v2)>(strings[ii + 1], v2);
        }
        if(retval) {
            output.emplace_back(v1, v2);
        } else {
            return false;
        }
    }
    return (!output.empty());
}

/// Conversion to a vector type using a particular single type as the conversion type
template <class T,
          class XC,
          enable_if_t<(expected_count<T>::value == expected_max_vector_size) && (expected_count<XC>::value == 1) &&
                          (type_count<XC>::value == 1),
                      detail::enabler> = detail::dummy>
bool lexical_conversion(const std::vector<std ::string> &strings, T &output) {
    bool retval = true;
    output.clear();
    output.reserve(strings.size());
    for(const auto &elem : strings) {

        output.emplace_back();
        retval = retval && lexical_assign<typename T::value_type, XC>(elem, output.back());
    }
    return (!output.empty()) && retval;
}
// This one is last since it can call other lexical_conversion functions
/// Lexical conversion if there is only one element but the conversion type is a vector
template <typename T,
          typename XC,
          enable_if_t<!is_tuple_like<T>::value && !is_vector<T>::value && is_vector<XC>::value, detail::enabler> =
              detail::dummy>
bool lexical_conversion(const std::vector<std ::string> &strings, T &output) {

    if(strings.size() > 1 || (!strings.empty() && !(strings.front().empty()))) {
        XC val;
        auto retval = lexical_conversion<XC, XC>(strings, val);
        output = T{val};
        return retval;
    }
    output = T{};
    return true;
}

/// function template for converting tuples if the static Index is greater than the tuple size
template <class T, class XC, std::size_t I>
inline typename std::enable_if<I >= type_count<T>::value, bool>::type tuple_conversion(const std::vector<std::string> &,
                                                                                       T &) {
    return true;
}
/// Tuple conversion operation
template <class T, class XC, std::size_t I>
    inline typename std::enable_if <
    I<type_count<T>::value, bool>::type tuple_conversion(const std::vector<std::string> &strings, T &output) {
    bool retval = true;
    if(strings.size() > I) {
        retval = retval && lexical_assign<typename std::tuple_element<I, T>::type,
                                          typename std::conditional<is_tuple_like<XC>::value,
                                                                    typename std::tuple_element<I, XC>::type,
                                                                    XC>::type>(strings[I], std::get<I>(output));
    }
    retval = retval && tuple_conversion<T, XC, I + 1>(strings, output);
    return retval;
}

/// Conversion for tuples
template <class T, class XC, enable_if_t<is_tuple_like<T>::value, detail::enabler> = detail::dummy>
bool lexical_conversion(const std::vector<std ::string> &strings, T &output) {
    static_assert(
        !is_tuple_like<XC>::value || type_count<T>::value == type_count<XC>::value,
        "if the conversion type is defined as a tuple it must be the same size as the type you are converting to");
    return tuple_conversion<T, XC, 0>(strings, output);
}

/// Lexical conversion of a vector types with type_size >2
template <class T,
          class XC,
          enable_if_t<expected_count<T>::value == expected_max_vector_size &&
                          expected_count<XC>::value == expected_max_vector_size && (type_count<XC>::value > 2),
                      detail::enabler> = detail::dummy>
bool lexical_conversion(const std::vector<std ::string> &strings, T &output) {
    bool retval = true;
    output.clear();
    std::vector<std::string> temp;
    std::size_t ii = 0;
    std::size_t icount = 0;
    std::size_t xcm = type_count<XC>::value;
    while(ii < strings.size()) {
        temp.push_back(strings[ii]);
        ++ii;
        ++icount;
        if(icount == xcm || temp.back().empty()) {
            if(static_cast<int>(xcm) == expected_max_vector_size) {
                temp.pop_back();
            }
            output.emplace_back();
            retval = retval && lexical_conversion<typename T::value_type, typename XC::value_type>(temp, output.back());
            temp.clear();
            if(!retval) {
                return false;
            }
            icount = 0;
        }
    }
    return retval;
}
/// Sum a vector of flag representations
/// The flag vector produces a series of strings in a vector,  simple true is represented by a "1",  simple false is
/// by
/// "-1" an if numbers are passed by some fashion they are captured as well so the function just checks for the most
/// common true and false strings then uses stoll to convert the rest for summing
template <typename T,
          enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value, detail::enabler> = detail::dummy>
void sum_flag_vector(const std::vector<std::string> &flags, T &output) {
    std::int64_t count{0};
    for(auto &flag : flags) {
        count += detail::to_flag_value(flag);
    }
    output = (count > 0) ? static_cast<T>(count) : T{0};
}

/// Sum a vector of flag representations
/// The flag vector produces a series of strings in a vector,  simple true is represented by a "1",  simple false is
/// by
/// "-1" an if numbers are passed by some fashion they are captured as well so the function just checks for the most
/// common true and false strings then uses stoll to convert the rest for summing
template <typename T,
          enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value, detail::enabler> = detail::dummy>
void sum_flag_vector(const std::vector<std::string> &flags, T &output) {
    std::int64_t count{0};
    for(auto &flag : flags) {
        count += detail::to_flag_value(flag);
    }
    output = static_cast<T>(count);
}

}  // namespace detail
}  // namespace CLI
