// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"

// [CLI11:public_includes:set]
#include <string>
#include <vector>
// [CLI11:public_includes:end]

#include "Macros.hpp"

namespace CLI {
// [CLI11:argv_hpp:verbatim]
namespace detail {
#ifdef _WIN32
/// Decode UTF-8 argv from GetCommandLineW into result.
/// Returns true on success; on failure error_msg is filled with a description
CLI11_INLINE bool compute_win32_argv(std::vector<std::string> &result, std::string &error_msg);
#endif
}  // namespace detail
// [CLI11:argv_hpp:end]
}  // namespace CLI

#ifndef CLI11_COMPILE
#include "impl/Argv_inl.hpp"  // IWYU pragma: export
#endif
