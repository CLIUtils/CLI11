#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <sstream>

#include "CLI/Macros.hpp"

// [CLI11:verbatim]
#ifdef __has_include
#if defined(CLI11_CPP17) && __has_include(<optional>)
#include <optional>
#define CLI11_OPTIONAL
namespace CLI {
using std::experimental::optional;
} // namespace CLI
#elif defined(CPP11_CPP14) && __has_include(<experimental/optional>)
#include <experimental/optional>
#define CLI11_OPTIONAL
namespace CLI {
using std::optional;
} // namespace CLI
#endif
#endif
// [CLI11:verbatim]

namespace CLI {

#ifdef CLI11_OPTIONAL

template <typename T> std::istream &operator>>(std::istream &in, optional<T> &val) {
    T v;
    in >> v;
    val = v;
    return in;
}

#endif

} // namespace CLI
