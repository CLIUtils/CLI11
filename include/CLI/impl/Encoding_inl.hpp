// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// This include is only needed for IDEs to discover symbols
#include <CLI/Encoding.hpp>
#include <CLI/Macros.hpp>

// [CLI11:public_includes:set]
#include <cstring>
#include <cwchar>
#include <locale>
#include <string>
#include <type_traits>
// [CLI11:public_includes:end]

namespace CLI {
// [CLI11:encoding_inl_hpp:verbatim]

namespace detail {

template <typename... T> struct false_t : std::false_type {};

CLI11_DIAGNOSTIC_PUSH
CLI11_DIAGNOSTIC_IGNORE_DEPRECATED

template <class T = int> CLI11_INLINE std::string narrow_impl(const wchar_t *str, std::size_t str_size) {
#if defined(CLI11_HAS_CODECVT) && CLI11_HAS_CODECVT > 0
#ifdef _WIN32
    static_assert(false_t<T>::value || sizeof(wchar_t) == 2, "cannot use narrow: wchar_t is expected to be UTF-16");
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().to_bytes(str, str + str_size);
#else
    static_assert(false_t<T>::value || sizeof(wchar_t) == 4, "cannot use narrow: wchar_t is expected to be UTF-32");
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(str, str + str_size);
#endif  // _WIN32
#else   // CLI11_HAS_CODECVT
    static_assert(false_t<T>::value, "cannot use narrow: <codecvt> not available");
#endif  // CLI11_HAS_CODECVT
}

template <class T = int> CLI11_INLINE std::wstring widen_impl(const char *str, std::size_t str_size) {
#if defined(CLI11_HAS_CODECVT) && CLI11_HAS_CODECVT > 0
#ifdef _WIN32
    static_assert(false_t<T>::value || sizeof(wchar_t) == 2, "cannot use widen: wchar_t is expected to be UTF-16");
    return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>().from_bytes(str, str + str_size);
#else
    static_assert(false_t<T>::value || sizeof(wchar_t) == 4, "cannot use widen: wchar_t is expected to be UTF-32");
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(str, str + str_size);
#endif  // _WIN32
#else   // CLI11_HAS_CODECVT
    static_assert(false_t<T>::value, "cannot use widen: <codecvt> not available");
#endif  // CLI11_HAS_CODECVT
}

CLI11_DIAGNOSTIC_POP

}  // namespace detail

CLI11_INLINE std::string narrow(const wchar_t *str, std::size_t str_size) { return detail::narrow_impl(str, str_size); }
CLI11_INLINE std::string narrow(const std::wstring &str) { return detail::narrow_impl(str.data(), str.size()); }
CLI11_INLINE std::string narrow(const wchar_t *str) { return detail::narrow_impl(str, std::wcslen(str)); }

CLI11_INLINE std::wstring widen(const char *str, std::size_t str_size) { return detail::widen_impl(str, str_size); }
CLI11_INLINE std::wstring widen(const std::string &str) { return detail::widen_impl(str.data(), str.size()); }
CLI11_INLINE std::wstring widen(const char *str) { return detail::widen_impl(str, std::strlen(str)); }

#ifdef CLI11_CPP17
CLI11_INLINE std::string narrow(std::wstring_view str) { return detail::narrow_impl(str.data(), str.size()); }
CLI11_INLINE std::wstring widen(std::string_view str) { return detail::widen_impl(str.data(), str.size()); }
#endif  // CLI11_CPP17

#if defined CLI11_HAS_FILESYSTEM && CLI11_HAS_FILESYSTEM > 0
CLI11_INLINE std::filesystem::path to_path(std::string_view str) {
    return std::filesystem::path{
#ifdef _WIN32
        widen(str)
#else
        str
#endif  // _WIN32
    };
}
#endif  // CLI11_HAS_FILESYSTEM

// [CLI11:encoding_inl_hpp:end]
}  // namespace CLI
