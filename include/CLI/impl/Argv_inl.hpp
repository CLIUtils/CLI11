// Copyright (c) 2017-2023, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// This include is only needed for IDEs to discover symbols
#include <CLI/Argv.hpp>

#include <CLI/Encoding.hpp>

// [CLI11:public_includes:set]
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
// [CLI11:public_includes:end]

// [CLI11:argv_inl_includes:verbatim]
#if defined(_WIN32)
#if !(defined(_AMD64_) || defined(_X86_) || defined(_ARM_))
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) ||           \
    defined(_M_AMD64)
#define _AMD64_
#elif defined(i386) || defined(__i386) || defined(__i386__) || defined(__i386__) || defined(_M_IX86)
#define _X86_
#elif defined(__arm__) || defined(_M_ARM) || defined(_M_ARMT)
#define _ARM_
#endif
#endif

// first
#ifndef NOMINMAX
// if NOMINMAX is already defined we don't want to mess with that either way
#define NOMINMAX
#include <windef.h>
#undef NOMINMAX
#else
#include <windef.h>
#endif

// second
#include <winbase.h>
// third
#include <processthreadsapi.h>
#include <shellapi.h>

#elif defined(__APPLE__)
#include <crt_externs.h>
#endif
// [CLI11:argv_inl_includes:end]

namespace CLI {
// [CLI11:argv_inl_hpp:verbatim]

namespace detail {

#ifdef __APPLE__
// Copy argc and argv as early as possible to avoid modification
static const std::vector<const char *> static_args = [] {
    static const std::vector<std::string> static_args_as_strings = [] {
        std::vector<std::string> args_as_strings;
        int argc = *_NSGetArgc();
        char **argv = *_NSGetArgv();

        args_as_strings.reserve(static_cast<size_t>(argc));
        for(size_t i = 0; i < static_cast<size_t>(argc); i++) {
            args_as_strings.push_back(argv[i]);
        }

        return args_as_strings;
    }();

    std::vector<const char *> static_args_result;
    static_args_result.reserve(static_args_as_strings.size());

    for(const auto &arg : static_args_as_strings) {
        static_args_result.push_back(arg.data());
    }

    return static_args_result;
}();
#endif

/// Command-line arguments, as passed in to this executable, converted to utf-8 on Windows.
CLI11_INLINE const std::vector<const char *> &args() {
    // This function uses initialization via lambdas extensively to take advantage of the thread safety of static
    // variable initialization [stmt.dcl.3]

#ifdef _WIN32
    static const std::vector<const char *> static_args = [] {
        static const std::vector<std::string> static_args_as_strings = [] {
            // On Windows, take arguments from GetCommandLineW and convert them to utf-8.
            std::vector<std::string> args_as_strings;
            int argc = 0;

            auto deleter = [](wchar_t **ptr) { LocalFree(ptr); };
            // NOLINTBEGIN(*-avoid-c-arrays)
            auto wargv =
                std::unique_ptr<wchar_t *[], decltype(deleter)>(CommandLineToArgvW(GetCommandLineW(), &argc), deleter);
            // NOLINTEND(*-avoid-c-arrays)

            if(wargv == nullptr) {
                throw std::runtime_error("CommandLineToArgvW failed with code " + std::to_string(GetLastError()));
            }

            args_as_strings.reserve(static_cast<size_t>(argc));
            for(size_t i = 0; i < static_cast<size_t>(argc); ++i) {
                args_as_strings.push_back(narrow(wargv[i]));
            }

            return args_as_strings;
        }();

        std::vector<const char *> static_args_result;
        static_args_result.reserve(static_args_as_strings.size());

        for(const auto &arg : static_args_as_strings) {
            static_args_result.push_back(arg.data());
        }

        return static_args_result;
    }();

    return static_args;

#elif defined(__APPLE__)

    return static_args;

#else
    static const std::vector<const char *> static_args = [] {
        static const std::vector<char> static_cmdline = [] {
            // On posix, retrieve arguments from /proc/self/cmdline, separated by null terminators.
            std::vector<char> cmdline;

            auto deleter = [](FILE *f) { std::fclose(f); };
            std::unique_ptr<FILE, decltype(deleter)> fp_unique(std::fopen("/proc/self/cmdline", "r"), deleter);
            FILE *fp = fp_unique.get();
            if(!fp) {
                throw std::runtime_error("could not open /proc/self/cmdline for reading");  // LCOV_EXCL_LINE
            }

            size_t size = 0;
            while(std::feof(fp) == 0) {
                cmdline.resize(size + 128);
                size += std::fread(cmdline.data() + size, 1, 128, fp);

                if(std::ferror(fp) != 0) {
                    throw std::runtime_error("error during reading /proc/self/cmdline");  // LCOV_EXCL_LINE
                }
            }
            cmdline.resize(size);

            return cmdline;
        }();

        std::size_t argc = static_cast<std::size_t>(std::count(static_cmdline.begin(), static_cmdline.end(), '\0'));
        std::vector<const char *> static_args_result;
        static_args_result.reserve(argc);

        for(auto it = static_cmdline.begin(); it != static_cmdline.end();
            it = std::find(it, static_cmdline.end(), '\0') + 1) {
            static_args_result.push_back(static_cmdline.data() + (it - static_cmdline.begin()));
        }

        return static_args_result;
    }();

    return static_args;
#endif
}

}  // namespace detail

CLI11_INLINE const char *const *argv() { return detail::args().data(); }
CLI11_INLINE int argc() { return static_cast<int>(detail::args().size()); }

// [CLI11:argv_inl_hpp:end]
}  // namespace CLI
