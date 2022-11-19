// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// This include is only needed for IDEs to discover symbols
#include <CLI/Argv.hpp>

#include <CLI/Encoding.hpp>

// [CLI11:public_includes:set]
#include <memory>
#include <string>
#include <vector>
// [CLI11:public_includes:set]

#ifdef _WIN32
// [CLI11:public_includes:set]
#include <processenv.h>
#include <shellapi.h>
// [CLI11:public_includes:set]
#endif  // _WIN32

namespace CLI {
// [CLI11:argv_inl_hpp:verbatim]

namespace detail {

/// Command-line arguments, as passed in to this executable, converted to utf-8 on Windows.
CLI11_INLINE const std::vector<const char *> &args() {
    // This function uses initialization via lambdas extensively to take advantage of the thread safety of static
    // variable initialization [stmt.dcl.3]

#ifdef _WIN32
    static const std::vector<const char *> result = [] {
        static const std::vector<std::string> args_as_strings = [] {
            // On Windows, take arguments from GetCommandLineW and convert them to utf-8.
            std::vector<std::string> result;
            int argc = 0;

            auto deleter = [](wchar_t **ptr) { LocalFree(ptr); };
            // NOLINTBEGIN(*-avoid-c-arrays)
            auto wargv =
                std::unique_ptr<wchar_t *[], decltype(deleter)>(CommandLineToArgvW(GetCommandLineW(), &argc), deleter);
            // NOLINTEND(*-avoid-c-arrays)

            if(wargv == nullptr) {
                throw std::runtime_error("CommandLineToArgvW failed with code " + std::to_string(GetLastError()));
            }

            result.reserve(static_cast<size_t>(argc));
            for(size_t i = 0; i < static_cast<size_t>(argc); ++i) {
                result.push_back(narrow(wargv[i]));
            }

            return result;
        }();

        std::vector<const char *> result;
        result.reserve(args_as_strings.size());

        for(const auto &arg : args_as_strings) {
            result.push_back(arg.data());
        }

        return result;
    }();

    return result;

#else
    static const std::vector<const char *> result = [] {
        static const std::vector<char> cmdline = [] {
            // On posix, retrieve arguments from /proc/self/cmdline, separated by null terminators.
            std::vector<char> result;

            auto deleter = [](FILE *f) { std::fclose(f); };
            std::unique_ptr<FILE, decltype(deleter)> fp_unique(std::fopen("/proc/self/cmdline", "r"), deleter);
            FILE *fp = fp_unique.get();
            if(!fp) {
                throw std::runtime_error("could not open /proc/self/cmdline for reading");
            }

            size_t size = 0;
            while(!std::feof(fp)) {
                result.resize(size + 128);
                size += std::fread(result.data() + size, 1, 128, fp);

                if(std::ferror(fp)) {
                    throw std::runtime_error("error during reading /proc/self/cmdline");
                }
            }
            result.resize(size);

            return result;
        }();

        std::size_t argc = static_cast<std::size_t>(std::count(cmdline.begin(), cmdline.end(), '\0'));
        std::vector<const char *> result;
        result.reserve(argc);

        for(auto it = cmdline.begin(); it != cmdline.end(); it = std::find(it, cmdline.end(), '\0') + 1) {
            result.push_back(cmdline.data() + (it - cmdline.begin()));
        }

        return result;
    }();

    return result;
#endif  // _WIN32
}

}  // namespace detail

CLI11_INLINE const char *const *argv() { return detail::args().data(); }
CLI11_INLINE int argc() { return static_cast<int>(detail::args().size()); }

// [CLI11:argv_inl_hpp:end]
}  // namespace CLI
