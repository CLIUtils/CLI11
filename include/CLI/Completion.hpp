// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"

// [CLI11:public_includes:set]
#include <cstdint>
#include <string>
#include <vector>
// [CLI11:public_includes:end]

#include "Macros.hpp"

namespace CLI {
// [CLI11:completion_hpp:verbatim]

/// Instructions passed alongside the candidates telling the shell what else it may do.
///
/// The values are a bit mask so a reply can carry several at once; each directive takes its bit
/// with the feature that emits it.
enum class CompletionDirective : std::uint8_t {
    /// No special handling: the shell falls back to its own file completion when nothing matched.
    Default = 0,
    /// The shell must not add filenames of its own to the candidates.
    NoFileComp = 2
};

/// A single thing the shell may insert on the command line.
///
/// The value is always a whole insertable token, never a fragment of one.
struct CompletionResult {
    std::string value;
};

/// Everything the binary has to say about one completion request.
struct CompletionReply {
    std::vector<CompletionResult> results{};
    CompletionDirective directive{CompletionDirective::Default};
};

/// Render a reply into the line-based text the generated shell scripts parse.
///
/// One candidate per line, followed by a final line holding a `:` and the directive as an integer.
CLI11_INLINE std::string format_completion_reply(const CompletionReply &reply);

namespace detail {

/// Produce the bash completion script for a program, parameterized by the activation variable.
CLI11_INLINE std::string completion_script_bash(const std::string &program_name, const std::string &env_var);

/// Turn an arbitrary program name into something usable as a shell function name.
CLI11_INLINE std::string completion_script_identifier(const std::string &program_name);

}  // namespace detail

// [CLI11:completion_hpp:end]
}  // namespace CLI

#ifndef CLI11_COMPILE
#include "impl/Completion_inl.hpp"  // IWYU pragma: export
#endif
