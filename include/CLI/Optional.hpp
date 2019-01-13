#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <istream>

#include "CLI/Macros.hpp"

// [CLI11:verbatim]
#ifdef __has_include

// You can explicitly enable or disable support
// by defining these to 1 or 0.
#if defined(CLI11_CPP17) && __has_include(<optional>) && \
     !defined(CLI11_STD_OPTIONAL)
#define CLI11_STD_OPTIONAL 1
#elif !defined(CLI11_STD_OPTIONAL)
#define CLI11_STD_OPTIONAL 0
#endif

#if defined(CLI11_CPP14) && __has_include(<experimental/optional>) && \
    !defined(CLI11_EXPERIMENTAL_OPTIONAL) \
    && (!defined(CLI11_STD_OPTIONAL) || CLI11_STD_OPTIONAL == 0)
#define CLI11_EXPERIMENTAL_OPTIONAL 1
#elif !defined(CLI11_EXPERIMENTAL_OPTIONAL)
#define CLI11_EXPERIMENTAL_OPTIONAL 0
#endif

#if __has_include(<boost/optional.hpp>) && !defined(CLI11_BOOST_OPTIONAL)
#include <boost/version.hpp>
#if BOOST_VERSION >= 105800
#define CLI11_BOOST_OPTIONAL 1
#endif
#elif !defined(CLI11_BOOST_OPTIONAL)
#define CLI11_BOOST_OPTIONAL 0
#endif

#endif

#if CLI11_STD_OPTIONAL
#include <optional>
#endif
#if CLI11_EXPERIMENTAL_OPTIONAL
#include <experimental/optional>
#endif
#if CLI11_BOOST_OPTIONAL
#include <boost/optional.hpp>
#endif
// [CLI11:verbatim]

namespace CLI {

#if CLI11_STD_OPTIONAL
template <typename T> std::istream &operator>>(std::istream &in, std::optional<T> &val) {
    T v;
    in >> v;
    val = v;
    return in;
}
#endif

#if CLI11_EXPERIMENTAL_OPTIONAL
template <typename T> std::istream &operator>>(std::istream &in, std::experimental::optional<T> &val) {
    T v;
    in >> v;
    val = v;
    return in;
}
#endif

#if CLI11_BOOST_OPTIONAL
template <typename T> std::istream &operator>>(std::istream &in, boost::optional<T> &val) {
    T v;
    in >> v;
    val = v;
    return in;
}
#endif

// Export the best optional to the CLI namespace
#if CLI11_STD_OPTIONAL
using std::optional;
#elif CLI11_EXPERIMENTAL_OPTIONAL
using std::experimental::optional;
#elif CLI11_BOOST_OPTIONAL
using boost::optional;
#endif

// This is true if any optional is found
#if CLI11_STD_OPTIONAL || CLI11_EXPERIMENTAL_OPTIONAL || CLI11_BOOST_OPTIONAL
#define CLI11_OPTIONAL 1
#endif

} // namespace CLI
