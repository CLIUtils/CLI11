#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

namespace CLI {

// Note that all code in CLI11 must be in a namespace, even if it just a define.

// The following version macro is very similar to the one in PyBind11

#if !defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#if __cplusplus >= 201402L
#define CLI11_CPP14
#if __cplusplus > 201402L /* Temporary: should be updated to >= the final C++17 value once known */
#define CLI11_CPP17
#endif
#endif
#elif defined(_MSC_VER)
// MSVC sets _MSVC_LANG rather than __cplusplus (supposedly until the standard is fully implemented)
#if _MSVC_LANG >= 201402L
#define CLI11_CPP14
#if _MSVC_LANG > 201402L && _MSC_VER >= 1910
#define CLI11_CPP17
#endif
#endif
#endif

} // namespace CLI
