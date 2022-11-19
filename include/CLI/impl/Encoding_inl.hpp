// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// This include is only needed for IDEs to discover symbols
#include <CLI/Encoding.hpp>

// [CLI11:encoding_inl_includes:verbatim]
#ifdef _WIN32
#include <cstring>
#include <cwchar>
#include <stdexcept>
#include <string>
#endif  // _WIN32
// [CLI11:encoding_inl_includes:end]

#ifdef _WIN32
#include "SlimWindowsH.hpp"
#endif  // _WIN32

namespace CLI {
// [CLI11:encoding_inl_hpp:verbatim]

#ifdef _WIN32
CLI11_INLINE std::string narrow(const wchar_t *str, std::size_t str_size) {
    std::string result;
    auto result_size = static_cast<std::size_t>(
        WideCharToMultiByte(CP_UTF8, 0, str, static_cast<int>(str_size), nullptr, 0, nullptr, nullptr));
    if(result_size == 0) {
        throw std::runtime_error("WinAPI function WideCharToMultiByte failed with code " +
                                 std::to_string(GetLastError()));
    }

    result.resize(result_size);

    auto ok = static_cast<bool>(WideCharToMultiByte(CP_UTF8,
                                                    0,
                                                    str,
                                                    static_cast<int>(str_size),
                                                    const_cast<char *>(result.data()),
                                                    static_cast<int>(result_size),
                                                    nullptr,
                                                    nullptr));
    if(!ok) {
        throw std::runtime_error("WinAPI function WideCharToMultiByte failed with code " +
                                 std::to_string(GetLastError()));
    }

    return result;
}

CLI11_INLINE std::wstring widen(const char *str, std::size_t str_size) {
    std::wstring result;
    auto result_size =
        static_cast<std::size_t>(MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(str_size), nullptr, 0));
    if(result_size == 0) {
        throw std::runtime_error("WinAPI function MultiByteToWideChar failed with code " +
                                 std::to_string(GetLastError()));
    }

    result.resize(result_size);

    auto ok = static_cast<bool>(MultiByteToWideChar(CP_UTF8,
                                                    0,
                                                    str,
                                                    static_cast<int>(str_size),
                                                    const_cast<wchar_t *>(result.data()),
                                                    static_cast<int>(result_size)));
    if(!ok) {
        throw std::runtime_error("WinAPI function MultiByteToWideChar failed with code " +
                                 std::to_string(GetLastError()));
    }

    return result;
}

CLI11_INLINE std::string narrow(const std::wstring &str) { return narrow(str.data(), str.size()); }
CLI11_INLINE std::string narrow(const wchar_t *str) { return narrow(str, std::wcslen(str)); }

CLI11_INLINE std::wstring widen(const std::string &str) { return widen(str.data(), str.size()); }
CLI11_INLINE std::wstring widen(const char *str) { return widen(str, std::strlen(str)); }

#ifdef CLI11_CPP17
CLI11_INLINE std::string narrow(std::wstring_view str) { return narrow(str.data(), str.size()); }
CLI11_INLINE std::wstring widen(std::string_view str) { return widen(str.data(), str.size()); }
#endif  // CLI11_CPP17
#endif  // _WIN32

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
