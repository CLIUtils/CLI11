// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <string>

#if defined CLI11_HAS_FILESYSTEM && CLI11_HAS_FILESYSTEM > 0
#include <filesystem>
#endif  // CLI11_HAS_FILESYSTEM

// "Hello Halló Привет 你好 👩‍🚀❤️"
static const uint8_t utf8_codeunits[] = {0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x48, 0x61, 0x6c, 0x6c, 0xc3, 0xb3, 0x20,
                                         0xd0, 0x9f, 0xd1, 0x80, 0xd0, 0xb8, 0xd0, 0xb2, 0xd0, 0xb5, 0xd1, 0x82, 0x20,
                                         0xe4, 0xbd, 0xa0, 0xe5, 0xa5, 0xbd, 0x20, 0xf0, 0x9f, 0x91, 0xa9, 0xe2, 0x80,
                                         0x8d, 0xf0, 0x9f, 0x9a, 0x80, 0xe2, 0x9d, 0xa4, 0xef, 0xb8, 0x8f};
static const std::string str(reinterpret_cast<const char *>(utf8_codeunits),
                             sizeof(utf8_codeunits) / sizeof(utf8_codeunits[0]));

#ifdef _WIN32
static const uint16_t utf16_codeunits[] = {0x0048, 0x0065, 0x006c, 0x006c, 0x006f, 0x0020, 0x0048, 0x0061,
                                           0x006c, 0x006c, 0x00f3, 0x0020, 0x041f, 0x0440, 0x0438, 0x0432,
                                           0x0435, 0x0442, 0x0020, 0x4f60, 0x597d, 0x0020, 0xd83d, 0xdc69,
                                           0x200d, 0xd83d, 0xde80, 0x2764, 0xfe0f};
static const std::wstring wstr(reinterpret_cast<const wchar_t *>(utf16_codeunits),
                               sizeof(utf16_codeunits) / sizeof(utf16_codeunits[0]));

#else
static const uint32_t utf32_codeunits[] = {
    0x00000048, 0x00000065, 0x0000006c, 0x0000006c, 0x0000006f, 0x00000020, 0x00000048, 0x00000061, 0x0000006c,
    0x0000006c, 0x000000f3, 0x00000020, 0x0000041f, 0x00000440, 0x00000438, 0x00000432, 0x00000435, 0x00000442,
    0x00000020, 0x00004f60, 0x0000597d, 0x00000020, 0x0001f469, 0x0000200d, 0x0001f680, 0x00002764, 0x0000fe0f};
static const std::wstring wstr(reinterpret_cast<const wchar_t *>(utf32_codeunits),
                               sizeof(utf32_codeunits) / sizeof(utf32_codeunits[0]));

#endif

// #14
TEST_CASE("Encoding: Widen", "[unicode]") {
    using CLI::widen;

    CHECK(wstr == widen(str));
    CHECK(wstr == widen(str.c_str()));
    CHECK(wstr == widen(str.c_str(), str.size()));
}

// #14
TEST_CASE("Encoding: Narrow", "[unicode]") {
    using CLI::narrow;

    CHECK(str == narrow(wstr));
    CHECK(str == narrow(wstr.c_str()));
    CHECK(str == narrow(wstr.c_str(), wstr.size()));
}

#if defined CLI11_HAS_FILESYSTEM && CLI11_HAS_FILESYSTEM > 0
// #14
TEST_CASE("Encoding: to_path roundtrip", "[unicode]") {
    using std::filesystem::path;

#ifdef _WIN32
    std::wstring native_str = CLI::widen(str);
#else
    std::string native_str = str;
#endif  // _WIN32

    CHECK(CLI::to_path(str).native() == native_str);
}

#endif  // CLI11_HAS_FILESYSTEM
