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
#include <utility>
#include <vector>
// [CLI11:public_includes:end]

#include "Macros.hpp"

namespace CLI {
// [CLI11:completion_hpp:verbatim]

class App;
class Option;

/// The version of the reply format that this build speaks.
///
/// A generated script exports it and the binary checks it, so a script that outlives the binary it was generated
/// against, such as one a distribution packaged as a static file, offers nothing rather than offering nonsense.
CLI11_MODULE_INLINE constexpr int CLI11_COMPLETE_PROTO_VERSION = 1;

/// Instructions passed alongside the candidates telling the shell what else it may do.
///
/// The values are a bit mask so a reply can carry several at once; each directive takes its bit
/// with the feature that emits it.
enum class CompletionDirective : std::uint8_t {
    /// Nothing is known about the value: the shell offers paths when there are no candidates.
    Default = 0,
    /// The request could not be answered, so the shell must ignore the reply entirely.
    Error = 1,
    /// The shell must not add filenames of its own to the candidates.
    NoFileComp = 2,
    /// The candidates arrive in a meaningful order, so the shell must not sort them.
    KeepOrder = 4,
    /// The value is a file name and the shell is to complete it.
    FilterFileExt = 8,
    /// The value is a directory name and the shell is to complete it. Together with FilterFileExt, either kind of
    /// path will do.
    FilterDirs = 16
};

/// Combine two directives, so that one reply can carry both
CLI11_MODULE_INLINE constexpr CompletionDirective operator|(CompletionDirective lhs, CompletionDirective rhs) {
    return static_cast<CompletionDirective>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

/// A kind of value that the shell completes better than the program can.
///
/// A path is the standing example: only the shell knows the working directory the user is typing in, and it already
/// knows how to walk one. An option that wants a path therefore says so and offers no candidates of its own.
enum class CompletionHint : std::uint8_t {
    /// Nothing is known about the shape of the value.
    None = 0,
    /// A file name.
    File,
    /// A directory name.
    Dir,
    /// Either a file or a directory.
    Path
};

/// What completion can learn from a Validator about the values it accepts.
///
/// A Validator is copied into a `Validator` base by `Option::check`, which slices any derived class away, so this is
/// how a validator that knows its accepted values hands them on: a plain member of the base that survives the copy.
struct CompletionMeta {
    /// The kind of value the validator accepts, when the shell is the one that can produce it.
    CompletionHint hint{CompletionHint::None};  // cppcheck-suppress unusedStructMember

    /// Every value the validator accepts, or an empty function when it does not enumerate them. A function that
    /// returns no values says that the validator accepts none.
    ///
    /// Read at completion time rather than stored as a list, because a validator may hold a reference to a set the
    /// program fills in after construction.
    std::function<std::vector<std::string>()> choices{};

    /// Whether the validator takes this text. Empty for a single validator, whose answer is a comparison against
    /// `choices` through `filters`; a combination of validators sets it, because each side compares by its own rules.
    std::function<bool(const std::string &)> accepts{};  // cppcheck-suppress unusedStructMember

    /// The normalisations the validator applies before it compares, such as `ignore_case`. Text typed so far matches
    /// a choice when it is a prefix of it as written or after any one of these has been applied to both.
    std::vector<std::function<std::string(std::string)>> filters{};  // cppcheck-suppress unusedStructMember
};

/// A single thing the shell may insert on the command line.
///
/// The value is always a whole insertable token, never a fragment of one. The description is shown beside the value
/// when the shell lists several candidates, and is dropped when there is only one to insert.
struct CompletionResult {
    std::string value;        // cppcheck-suppress unusedStructMember
    std::string description;  // cppcheck-suppress unusedStructMember
};

/// Everything the binary has to say about one completion request.
struct CompletionReply {
    std::vector<CompletionResult> results{};  // cppcheck-suppress unusedStructMember

    /// The leading part of the word being completed that is not the value, `--file=` at `--file=/et`.
    ///
    /// Candidates are whole tokens, so every one of them begins with it, but a shell rarely replaces a whole token:
    /// bash has already broken the word at the `=` and puts back only what follows it. Only the binary knows how much
    /// of the word is the option name, so it says so here rather than leaving each script to guess.
    std::string prefix{};  // cppcheck-suppress unusedStructMember

    CompletionDirective directive{CompletionDirective::Default};  // cppcheck-suppress unusedStructMember
};

/// Render a reply into the line-based text the generated shell scripts parse.
///
/// One candidate per line as `value` or `value<TAB>description`, then a `:prefix=` line when there is a prefix, then a
/// final line holding a `:` and the directive as an integer. Descriptions are cut at their first line break; every
/// field is escaped.
CLI11_INLINE std::string format_completion_reply(const CompletionReply &reply);

namespace detail {

/// Whether the Validator a meta describes takes this text, as far as its listed values can say. One that lists no
/// values is taken to accept it.
CLI11_INLINE bool completion_meta_accepts(const CompletionMeta &meta, const std::string &text);

/// What a completion request knows after reading the words in front of the cursor.
///
/// Completion parses nothing, so this holds the parts of the parser's state that decide what the next word can be.
struct CompletionWalk {
    /// A command the line has entered and not left, and whether it has seen the `--` marker
    struct Frame {
        const App *app;
        bool positional_only;
    };
    /// The commands the line is inside, the innermost last. Empty when the line cannot be read any further.
    std::vector<Frame> frames{};  // cppcheck-suppress unusedStructMember
    /// How many items each option has been given, by name or as a positional
    std::vector<std::pair<const Option *, int>> counts{};  // cppcheck-suppress unusedStructMember
    /// The option that still takes values when the cursor is reached, and how many items it has from this use
    const Option *awaiting{nullptr};  // cppcheck-suppress unusedStructMember
    int awaiting_items{0};            // cppcheck-suppress unusedStructMember
    /// Whether the word at the cursor is one of the values `awaiting` must have, which is taken whatever it looks like
    bool awaiting_minimum{false};  // cppcheck-suppress unusedStructMember

    /// The number of items the option has been given
    int &count(const Option *opt) {
        for(std::pair<const Option *, int> &entry : counts) {
            if(entry.first == opt)
                return entry.second;
        }
        counts.emplace_back(opt, 0);
        return counts.back().second;
    }
};

/// Combine what two Validators say about their values into what satisfies both of them
///
/// The intersection when both enumerate their values, and whichever one does when only one of them does. A Validator
/// that does not enumerate its values still constrains them, but the only way to apply that constraint would be to run
/// it, and completion does not run user code. Two hints cannot be intersected at all, so the left one wins.
CLI11_INLINE std::shared_ptr<const CompletionMeta>
intersect_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                          const std::shared_ptr<const CompletionMeta> &rhs);

/// Combine what two Validators say about their values into what satisfies either of them
///
/// The union when both enumerate their values. When only one does the answer is unbounded, so what comes back is that
/// one's values: a subset of what is acceptable rather than the whole of it. Hints do not union either, so again the
/// left one wins.
CLI11_INLINE std::shared_ptr<const CompletionMeta>
unite_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                      const std::shared_ptr<const CompletionMeta> &rhs);

/// What a Validator that only knows the kind of its value, and none of the values, tells completion
CLI11_INLINE std::shared_ptr<const CompletionMeta> completion_hint_meta(CompletionHint hint);

/// Turn what the shell scripts would otherwise read as structure (`\` `\t` `\n` `\r`, and a leading `:`) into the
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
