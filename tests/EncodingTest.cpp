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

#ifdef _WIN32
// #14
TEST_CASE("Encoding: Widen", "[unicode]") {
    using CLI::widen;

    std::string str = "Hello Halló Привет 你好 👩‍🚀❤️";
    std::wstring wstr = L"Hello Halló Привет 你好 👩‍🚀❤️";

    for(auto &c : wstr) {
        std::cout << std::hex << std::setfill('0') << std::setw(4) << static_cast<unsigned>(c) << " ";
    }
    std::cout << "\n";

    auto wstr2 = widen(str);
    for(auto &c : wstr2) {
        std::cout << std::hex << std::setfill('0') << std::setw(4) << static_cast<unsigned>(c) << " ";
    }
    std::cout << "\n";

    CHECK(wstr == widen(str));
    CHECK(wstr == widen(str.c_str()));
    CHECK(wstr == widen(str.c_str(), str.size()));
    FAIL();
}

// #14
TEST_CASE("Encoding: Narrow", "[unicode]") {
    using CLI::narrow;

    std::string str = "Hello Halló Привет 你好 👩‍🚀❤️";
    std::wstring wstr = L"Hello Halló Привет 你好 👩‍🚀❤️";

    for(auto &c : str) {
        std::cout << std::hex << std::setfill('0') << std::setw(2)
                  << static_cast<unsigned>(reinterpret_cast<unsigned char &>(c)) << " ";
    }
    std::cout << "\n";

    auto str2 = narrow(wstr);
    for(auto &c : str2) {
        std::cout << std::hex << std::setfill('0') << std::setw(2)
                  << static_cast<unsigned>(reinterpret_cast<unsigned char &>(c)) << " ";
    }
    std::cout << "\n";

    CHECK(str == narrow(wstr));
    CHECK(str == narrow(wstr.c_str()));
    CHECK(str == narrow(wstr.c_str(), wstr.size()));
}
#endif  // _WIN32

#if defined CLI11_HAS_FILESYSTEM && CLI11_HAS_FILESYSTEM > 0
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
