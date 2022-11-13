// Copyright (c) 2017-2022, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <CLI/Macros.hpp>

// [CLI11:public_includes:set]
#include <vector>
// [CLI11:public_includes:set]

namespace CLI {
// [CLI11:config_hpp:verbatim]

namespace detail {

/// Command-line arguments, as passed in to this executable, converted to utf-8 on Windows.
CLI11_INLINE const std::vector<const char *> &args();

}  // namespace detail

/// argc as passed in to this executable.
CLI11_INLINE int argc();

/// argv as passed in to this executable, converted to utf-8 on Windows.
CLI11_INLINE const char *const *argv();

// [CLI11:config_hpp:end]
}  // namespace CLI

#ifndef CLI11_COMPILE
#include "impl/Argv_inl.hpp"
#endif
