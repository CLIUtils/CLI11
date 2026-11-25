// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"

// [CLI11:public_includes:set]
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
// [CLI11:public_includes:end]

#include "Macros.hpp"

namespace CLI {
// [CLI11:completion_hpp:verbatim]

class App;
class Option;

/// The version of the reply format.
///
/// A generated script exports it and the binary checks it. A script can outlive its binary, for example as a static
/// file in a distribution package. On a mismatch, the script offers no candidates.
CLI11_MODULE_INLINE constexpr int CLI11_COMPLETE_PROTO_VERSION = 1;

/// Instructions for the shell, sent with the candidates.
///
/// The values are a bit mask, so a reply can hold more than one.
enum class CompletionDirective : std::uint8_t {
    /// The value is unknown. The shell offers paths when there are no candidates.
    Default = 0,
    /// The request has no answer. The shell must ignore the reply.
    Error = 1,
    /// The shell must not add file names to the candidates.
    NoFileComp = 2,
    /// The order of the candidates has a meaning. The shell must not sort them.
    KeepOrder = 4,
    /// The value is a file name. The shell completes it.
    Files = 8,
    /// The value is a directory name. The shell completes it. With Files, the value is a file or a directory.
    Dirs = 16
};

/// Combine two directives
CLI11_MODULE_INLINE constexpr CompletionDirective operator|(CompletionDirective lhs, CompletionDirective rhs) {
    return static_cast<CompletionDirective>(static_cast<std::uint8_t>(lhs) | static_cast<std::uint8_t>(rhs));
}

/// A kind of value that the shell completes better than the program can.
///
/// Only the shell knows the working directory of the user. An option that takes a path sets a hint and gives no
/// candidates.
enum class CompletionHint : std::uint8_t {
    /// The kind of value is unknown.
    None = 0,
    /// A file name.
    File,
    /// A directory name.
    Dir,
    /// Either a file or a directory.
    Path
};

/// What a Validator tells completion about the values it accepts.
///
/// `Option::check` copies a Validator into a `Validator` base, which slices a derived class. This struct is a member of
/// the base, so the copy keeps it.
struct CompletionMeta {
    /// The kind of value the validator accepts, when the shell completes it.
    CompletionHint hint{CompletionHint::None};  // cppcheck-suppress unusedStructMember

    /// The values the validator accepts. An empty function means the validator does not list its values. A function
    /// that returns no values means the validator accepts none.
    ///
    /// Completion calls it for each request, because a validator can hold a reference to a set that the program fills
    /// later.
    std::function<std::vector<std::string>()> choices{};

    /// Whether the validator accepts this text. Empty for a single validator: completion compares the text with
    /// `choices` through `filters`. A combination sets it, because each side has its own comparison rules.
    std::function<bool(const std::string &)> accepts{};  // cppcheck-suppress unusedStructMember

    /// The normalisations the validator applies before it compares, such as `ignore_case`. Typed text matches a choice
    /// when it is a prefix of the choice, as written or after one filter is applied to both.
    std::vector<std::function<std::string(std::string)>> filters{};  // cppcheck-suppress unusedStructMember

    /// Whether the validator also accepts values that are not in `choices`, as `IsMember(...) | ExistingFile` does.
    /// The hint then applies together with the choices.
    bool open{false};  // cppcheck-suppress unusedStructMember
};

/// One candidate that the shell can insert on the command line.
///
/// The value is a full token. The shell shows the description when it lists more than one candidate.
struct CompletionResult {
    std::string value;        // cppcheck-suppress unusedStructMember
    std::string description;  // cppcheck-suppress unusedStructMember
};

/// The reply to one completion request.
struct CompletionReply {
    std::vector<CompletionResult> results{};  // cppcheck-suppress unusedStructMember

    /// The part of the completed word that is not the value: `--file=` for `--file=/et`.
    ///
    /// Each candidate is a full token and starts with the prefix. bash splits the word at the `=` and replaces only the
    /// text after it. Only the binary knows where the option name ends, so it sends the prefix to the script.
    std::string prefix{};  // cppcheck-suppress unusedStructMember

    CompletionDirective directive{CompletionDirective::Default};  // cppcheck-suppress unusedStructMember
};

/// Render a reply as the line-based text that the generated shell scripts parse.
///
/// One candidate per line as `value` or `value<TAB>description`. Then a `:prefix=` line when there is a prefix. Then a
/// last line with a `:` and the directive as an integer. Descriptions are cut at the first line break. Each field is
/// escaped.
CLI11_INLINE std::string format_completion_reply(const CompletionReply &reply);

namespace detail {

/// Whether the Validator that a meta describes accepts this text. A meta that lists no values accepts all text.
CLI11_INLINE bool completion_meta_accepts(const CompletionMeta &meta, const std::string &text);

/// The state of a completion request after it reads the words before the cursor.
///
/// Completion does not parse. This struct holds the parser state that decides what the next word can be.
struct CompletionWalk {
    /// A command the line is in, and whether the line has the `--` marker there
    struct Frame {
        const App *app;
        bool positional_only;
    };
    /// The commands the line is in, the innermost last.
    std::vector<Frame> frames{};  // cppcheck-suppress unusedStructMember
    /// The number of items each option has, given by name or as a positional
    std::map<const Option *, int> counts{};  // cppcheck-suppress unusedStructMember
    /// The option that still takes values at the cursor, and the number of items it has from this use
    const Option *awaiting{nullptr};  // cppcheck-suppress unusedStructMember
    int awaiting_items{0};            // cppcheck-suppress unusedStructMember
    /// Whether the word at the cursor is a required value of `awaiting`. Such a word is a value whatever its shape.
    bool awaiting_minimum{false};  // cppcheck-suppress unusedStructMember
    /// True when the rest of the line cannot be read. The reply then has no candidates.
    bool halted{false};  // cppcheck-suppress unusedStructMember
    /// The number of subcommands each command has entered, as parsed_subcommands_ counts them
    std::map<const App *, std::size_t> entered{};  // cppcheck-suppress unusedStructMember
};

/// Combine the metas of two Validators into the meta of `A & B`.
///
/// When both list values, the result is the intersection. When one lists values, the result is that list: only a run of
/// the other Validator can apply its constraint, and completion does not run user code. Two hints cannot be
/// intersected, so the left hint wins.
CLI11_INLINE std::shared_ptr<const CompletionMeta>
intersect_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                          const std::shared_ptr<const CompletionMeta> &rhs);

/// Combine the metas of two Validators into the meta of `A | B`.
///
/// When both list values, the result is the union. When one lists values, the result keeps them and is `open`. A file
/// hint and a directory hint make a path hint.
CLI11_INLINE std::shared_ptr<const CompletionMeta>
unite_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                      const std::shared_ptr<const CompletionMeta> &rhs);

/// A command line as bash gives it, split into the words a program would receive
struct CompletionLine {
    /// The words with quotes and escapes removed. Word 0 is the program. The cursor word ends at the cursor.
    std::vector<std::string> words{};  // cppcheck-suppress unusedStructMember
    /// The index of the word the cursor is in
    std::size_t cursor{0};  // cppcheck-suppress unusedStructMember
    /// How much of the cursor word readline does not replace: the text in front of a quote that is still open, or
    /// else the text up to the last break character that is not quoted
    std::size_t kept{0};  // cppcheck-suppress unusedStructMember
};

/// Split a line as bash does. `point` is the byte offset of the cursor (COMP_POINT) and `breaks` is COMP_WORDBREAKS.
CLI11_INLINE CompletionLine split_completion_line(const std::string &line,
                                                  std::size_t point,
                                                  const std::string &breaks);

/// Render a reply for the bash script, which replaces only the part of `word` after its first `kept` characters.
///
/// The kept text is removed from each candidate. In place of the `:prefix=` line there are up to three lines for path
/// completion: `:cur=` is the value typed so far, `:strip=` is text to remove from the front of each path, and
/// `:lead=` is text to put in front of each path.
CLI11_INLINE std::string format_completion_reply_bash(CompletionReply reply, const std::string &word, std::size_t kept);

/// The directive that asks the shell for the kind of path in a hint. Default when the hint is None.
CLI11_INLINE CompletionDirective completion_hint_directive(CompletionHint hint);

/// The meta of a Validator that knows the kind of its value but not the values
CLI11_INLINE std::shared_ptr<const CompletionMeta> completion_hint_meta(CompletionHint hint);

/// Escape the characters that the shell scripts read as structure: `\`, `\t`, `\n`, `\r`, and a leading `:`. The
/// generated scripts reverse the backslash sequences.
CLI11_INLINE std::string escape_completion_field(const std::string &field);

/// Produce the bash completion script for a program. `env_var` is the activation variable.
CLI11_INLINE std::string completion_script_bash(const std::string &program_name, const std::string &env_var);

/// Convert a program name to text that is valid in a shell function name.
CLI11_INLINE std::string completion_script_identifier(const std::string &program_name);

}  // namespace detail

// [CLI11:completion_hpp:end]
}  // namespace CLI

#ifndef CLI11_COMPILE
#include "impl/Completion_inl.hpp"  // IWYU pragma: export
#endif
