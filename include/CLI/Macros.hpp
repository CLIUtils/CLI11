// Copyright (c) 2017-2021, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// [CLI11:macros_hpp:verbatim]

// The following version macro is very similar to the one in pybind11
#if !(defined(_MSC_VER) && __cplusplus == 199711L) && !defined(__INTEL_COMPILER)
#if __cplusplus >= 201402L
#define CLI11_CPP14
#if __cplusplus >= 201703L
#define CLI11_CPP17
#if __cplusplus > 201703L
#define CLI11_CPP20
#endif
#endif
#endif
#elif defined(_MSC_VER) && __cplusplus == 199711L
// MSVC sets _MSVC_LANG rather than __cplusplus (supposedly until the standard is fully implemented)
// Unless you use the /Zc:__cplusplus flag on Visual Studio 2017 15.7 Preview 3 or newer
#if _MSVC_LANG >= 201402L
#define CLI11_CPP14
#if _MSVC_LANG > 201402L && _MSC_VER >= 1910
#define CLI11_CPP17
#if __MSVC_LANG > 201703L && _MSC_VER >= 1910
#define CLI11_CPP20
#endif
#endif
#endif
#endif

#if defined(CLI11_CPP14)
#define CLI11_DEPRECATED(reason) [[deprecated(reason)]]
#elif defined(_MSC_VER)
#define CLI11_DEPRECATED(reason) __declspec(deprecated(reason))
#else
#define CLI11_DEPRECATED(reason) __attribute__((deprecated(reason)))
#endif

// [CLI11:macros_hpp:end]
