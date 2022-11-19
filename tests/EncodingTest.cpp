// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <string>
#ifdef CLI11_HAS_FILESYSTEM
#include <filesystem>
#endif  // CLI11_HAS_FILESYSTEM

#ifdef _WIN32
// #14
TEST_CASE("Encoding: Wide strings roundtrip", "[unicode]") {
    using CLI::narrow;
    using CLI::widen;

    std::string str = "Hello Halló Привет 你好 👩‍🚀❤️";

    std::wstring wstr = widen(str);
    CHECK(wstr == widen(str.c_str()));
    CHECK(wstr == widen(str.c_str(), str.size()));

    std::string roundtrip_str = narrow(wstr);
    CHECK(roundtrip_str == narrow(wstr.c_str()));
    CHECK(roundtrip_str == narrow(wstr.c_str(), wstr.size()));

    CHECK(str == roundtrip_str);
}
#endif  // _WIN32

#ifdef CLI11_HAS_FILESYSTEM
// #14
TEST_CASE("Encoding: to_path roundtrip", "[unicode]") {
    using std::filesystem::path;

    std::string str = "Hello Halló Привет 你好 👩‍🚀❤️";

#ifdef _WIN32
    std::wstring native_str = CLI::widen(str);
#else
    std::string native_str = str;
#endif  // _WIN32

    CHECK(CLI::to_path(str).native() == native_str);
}
#endif  // CLI11_HAS_FILESYSTEM
