// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

enum class shell { cmd, powershell, bash };

/**
 * @brief Execute an `executable` with arguments contained in `args_file`.
 *
 * Useful to perform a closer-to-real-world test with system encodings affecting the arguments.
 */
template <shell S> void execute_with(const char *executable, const char *args_file) {
    static_assert(false && (S == S), "this shell is not supported on this operating system");
    (void)executable;
    (void)args_file;
}

#ifdef _WIN32
template <> void execute_with<shell::cmd>(const char *executable, const char *args_file);
template <> void execute_with<shell::powershell>(const char *executable, const char *args_file);
#else
template <> void execute_with<shell::bash>(const char *executable, const char *args_file);
#endif  // _WIN32
