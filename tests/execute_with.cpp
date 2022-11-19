// Copyright (c) 2017-2021, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "execute_with.hpp"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <synchapi.h>
#undef WIN32_LEAN_AND_MEAN
#endif  // _WIN32

#ifdef CLI11_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

template <typename F> struct scope_guard_t {
    F closure;

    explicit scope_guard_t(F closure_) : closure(closure_) {}
    ~scope_guard_t() { closure(); }
};

template <typename F> CLI11_NODISCARD scope_guard_t<F> scope_guard(F &&closure) {
    return scope_guard_t<F>{std::forward<F>(closure)};
}

#ifdef _WIN32

void winapi_execute(const char *executable, const char *commandline) {
    SHELLEXECUTEINFOA info{};
    info.cbSize = sizeof(SHELLEXECUTEINFO);
    info.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_NO_CONSOLE | SEE_MASK_FLAG_NO_UI;
    info.lpVerb = "open";
    info.nShow = SW_HIDE;

    info.lpFile = executable;
    info.lpParameters = commandline;

    // Do not leak resource handle on exception
    auto sg = scope_guard([&]() {
        if(info.hProcess != nullptr) {
            CloseHandle(info.hProcess);
        }
    });

    auto ok = static_cast<bool>(ShellExecuteExA(&info));
    if(!ok) {
        auto error_code = GetLastError();
        std::string error = "winapi_exec: ShellExecuteExA produced error code " + std::to_string(error_code);
        throw std::runtime_error(error.c_str());
    }

    auto stop_reason = WaitForSingleObject(info.hProcess, 1000 * 60);
    switch(stop_reason) {
    case WAIT_OBJECT_0:
        break;
    case WAIT_TIMEOUT:
        throw std::runtime_error("winapi_exec: executable runtime exceeded 60s");
    case WAIT_FAILED: {
        auto error_code = GetLastError();
        std::string error = "winapi_exec: WaitForSingleObject produced error code " + std::to_string(error_code);
        throw std::runtime_error(error.c_str());
    }
    default:
        throw std::runtime_error("winapi_exec: WaitForSingleObject produced an unknown error");
    }
}

template <> void execute_with<shell::cmd>(const char *executable_c_str, const char *args_file_c_str) {
    std::string executable = executable_c_str;
    std::string args_file = args_file_c_str;
    std::replace(executable.begin(), executable.end(), '/', '\\');
    std::replace(args_file.begin(), args_file.end(), '/', '\\');

    std::string commandline =
        R"lit(/C "for /F "usebackq delims=" %X in (")lit" + args_file + "\") do @\"" + executable + "\" %X \"";

    winapi_execute("cmd.exe", commandline.c_str());
}

template <> void execute_with<shell::powershell>(const char *executable_c_str, const char *args_file_c_str) {
    std::string executable = executable_c_str;
    std::string args_file = args_file_c_str;
    std::replace(executable.begin(), executable.end(), '/', '\\');
    std::replace(args_file.begin(), args_file.end(), '/', '\\');

    std::string commandline = R"lit(-NoProfile -NonInteractive -Command "&\")lit" + executable +
                              R"lit(\" $($(Get-Content \")lit" + args_file +
                              R"lit(\") -Split ' (?=(?:[^\"]|\"[^\"]*\")*$)')")lit";

    winapi_execute("powershell.exe", commandline.c_str());
}

#else

#include <iostream>

template <> void execute_with<shell::bash>(const char *executable, const char *args_file) {
    std::string commandline = std::string{"/usr/bin/env bash -c '\""} + executable + "\" \"$(< " + args_file + ")\"'";
    std::system(commandline.c_str());
}

#endif  // _WIN32
