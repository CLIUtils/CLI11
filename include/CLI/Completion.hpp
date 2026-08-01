// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"

// [CLI11:public_includes:set]
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
// [CLI11:public_includes:end]

#include "Macros.hpp"

namespace CLI {
// [CLI11:completion_hpp:verbatim]

/// The version of the reply format that this build speaks.
///
/// A generated script exports it and the binary checks it, so a script that outlives the binary it was generated
/// against -- one a distribution packaged as a static file -- offers nothing rather than offering nonsense.
CLI11_MODULE_INLINE constexpr int CLI11_COMPLETE_PROTO_VERSION = 1;

/// Instructions passed alongside the candidates telling the shell what else it may do.
///
/// The values are a bit mask so a reply can carry several at once; each directive takes its bit
/// with the feature that emits it.
enum class CompletionDirective : std::uint8_t {
    /// No special handling: the shell falls back to its own file completion when nothing matched.
    Default = 0,
    /// The request could not be answered, so the shell must ignore the reply entirely.
    Error = 1,
    /// The shell must not add filenames of its own to the candidates.
    NoFileComp = 2,
    /// The candidates arrive in a meaningful order, so the shell must not sort them.
    KeepOrder = 4
};

/// Combine two directives, so that one reply can carry both
CLI11_MODULE_INLINE constexpr CompletionDirective operator|(CompletionDirective lhs, CompletionDirective rhs) {
    return static_cast<CompletionDirective>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

/// What completion can learn from a Validator about the values it accepts.
///
/// A Validator is copied into a `Validator` base by `Option::check`, which slices any derived class away, so this is
/// how a validator that knows its accepted values hands them on: a plain member of the base that survives the copy.
struct CompletionMeta {
    /// Every value the validator accepts, or empty when it does not enumerate them.
    ///
    /// Read at completion time rather than stored as a list, because a validator may hold a reference to a set the
    /// program fills in after construction.
    std::function<std::vector<std::string>()> choices{};
};

/// A single thing the shell may insert on the command line.
///
/// The value is always a whole insertable token, never a fragment of one. The description is shown beside the value
/// when the shell lists several candidates, and is dropped when there is only one to insert.
struct CompletionResult {
    std::string value;
    std::string description;
};

/// Everything the binary has to say about one completion request.
struct CompletionReply {
    std::vector<CompletionResult> results{};
    CompletionDirective directive{CompletionDirective::Default};
};

/// Render a reply into the line-based text the generated shell scripts parse.
///
/// One candidate per line as `value` or `value<TAB>description`, followed by a final line holding a `:` and the
/// directive as an integer. Descriptions are cut at their first line break; both fields are escaped.
CLI11_INLINE std::string format_completion_reply(const CompletionReply &reply);

namespace detail {

/// Combine what two Validators say about their values into what satisfies both of them
///
/// The intersection when both enumerate their values, and whichever one does when only one of them does -- a Validator
/// that does not enumerate its values still constrains them, but the only way to apply that constraint would be to run
/// it, and completion does not run user code.
CLI11_INLINE std::shared_ptr<const CompletionMeta>
intersect_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                          const std::shared_ptr<const CompletionMeta> &rhs);

/// Combine what two Validators say about their values into what satisfies either of them
///
/// The union when both enumerate their values. When only one does the answer is unbounded, so what comes back is that
/// one's values: a subset of what is acceptable rather than the whole of it.
CLI11_INLINE std::shared_ptr<const CompletionMeta>
unite_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                      const std::shared_ptr<const CompletionMeta> &rhs);

/// Turn what the shell scripts would otherwise read as structure -- `\` `\t` `\n` `\r`, and a leading `:` -- into the
/// backslash sequences the generated scripts turn back
CLI11_INLINE std::string escape_completion_field(const std::string &field);

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
