// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"

// This include is only needed for IDEs to discover symbols
#include "../Completion.hpp"

// [CLI11:public_includes:set]
#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
// [CLI11:public_includes:end]

#include "../StringTools.hpp"

namespace CLI {
// [CLI11:completion_inl_hpp:verbatim]

namespace detail {

/// The reply text with the given `:name=value` lines between the candidates and the directive
inline std::string format_completion_lines(const CompletionReply &reply,
                                           const std::vector<std::pair<std::string, std::string>> &fields) {
    std::string out;
    for(const CompletionResult &result : reply.results) {
        out += escape_completion_field(result.value);

        // The shell shows one line, so the description is cut at the first line break.
        const std::string description = result.description.substr(0, result.description.find_first_of("\r\n"));
        if(!description.empty()) {
            out += '\t';
            out += escape_completion_field(description);
        }
        out += '\n';
    }
    // The directive line stays last, so a script can detect a truncated reply.
    for(const std::pair<std::string, std::string> &field : fields) {
        if(!field.second.empty())
            out += ":" + field.first + "=" + escape_completion_field(field.second) + "\n";
    }
    out += ':';
    out += std::to_string(static_cast<int>(reply.directive));
    out += '\n';
    return out;
}

}  // namespace detail

CLI11_INLINE std::string format_completion_reply(const CompletionReply &reply) {
    return detail::format_completion_lines(reply, {{"prefix", reply.prefix}});
}

namespace detail {

/// The common part of the two combinations: the filters of both sides, and the left hint if there is one. `both` is set
/// to true when each side lists choices. The caller then combines the choices.
inline std::shared_ptr<CompletionMeta>
merge_completion_meta(const CompletionMeta &lhs, const CompletionMeta &rhs, bool &both) {
    auto meta = std::make_shared<CompletionMeta>();
    // Typed text can match a choice through a filter of either side, so all filters apply.
    meta->filters = lhs.filters;
    meta->filters.insert(meta->filters.end(), rhs.filters.begin(), rhs.filters.end());
    // "A file" and "a directory" cannot be intersected, so the left hint wins. `ExistingFile & size_check` then
    // completes files. In a union, a hint is a set too large to list, so the result keeps the hint.
    meta->hint = (lhs.hint != CompletionHint::None) ? lhs.hint : rhs.hint;
    both = lhs.choices && rhs.choices;
    if(!both) {
        // A side that lists no values accepts all values, so the other side gives the answer.
        const CompletionMeta &listed = lhs.choices ? lhs : rhs;
        meta->choices = listed.choices;
        meta->accepts = listed.accepts;
        meta->open = listed.open;
        if(!meta->accepts && listed.choices)
            meta->filters = listed.filters;
    }
    return meta;
}

CLI11_INLINE bool completion_meta_accepts(const CompletionMeta &meta, const std::string &text) {
    if(meta.open)
        return true;
    if(meta.accepts)
        return meta.accepts(text);
    if(!meta.choices)
        return true;
    for(const std::string &choice : meta.choices()) {
        if(choice == text)
            return true;
        for(const std::function<std::string(std::string)> &filter : meta.filters) {
            if(filter(choice) == filter(text))
                return true;
        }
    }
    return false;
}

CLI11_INLINE std::shared_ptr<const CompletionMeta>
intersect_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                          const std::shared_ptr<const CompletionMeta> &rhs) {
    if(!lhs)
        return rhs;
    if(!rhs)
        return lhs;

    bool both = false;
    auto meta = merge_completion_meta(*lhs, *rhs, both);
    if(both) {
        meta->choices = [lhs, rhs]() {
            // The left order is kept, because it is the order of the declared set. The right side uses its own
            // comparison, so a value that the two sides spell differently is still in both.
            std::vector<std::string> out;
            for(const std::string &choice : lhs->choices()) {
                if(completion_meta_accepts(*rhs, choice))
                    out.push_back(choice);
            }
            return out;
        };
        meta->accepts = [lhs, rhs](const std::string &text) {
            return completion_meta_accepts(*lhs, text) && completion_meta_accepts(*rhs, text);
        };
        meta->open = lhs->open && rhs->open;
    }
    return meta;
}

CLI11_INLINE std::shared_ptr<const CompletionMeta>
unite_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                      const std::shared_ptr<const CompletionMeta> &rhs) {
    if(!lhs)
        return rhs;
    if(!rhs)
        return lhs;

    bool both = false;
    auto meta = merge_completion_meta(*lhs, *rhs, both);
    if(lhs->hint != CompletionHint::None && rhs->hint != CompletionHint::None && lhs->hint != rhs->hint)
        meta->hint = CompletionHint::Path;
    // The side that lists no values accepts values that the other side does not list.
    meta->open = static_cast<bool>(meta->choices);
    if(both) {
        meta->open = lhs->open || rhs->open;
        meta->choices = [lhs, rhs]() {
            std::vector<std::string> out = lhs->choices();
            for(const std::string &choice : rhs->choices()) {
                // Offer a value that both sides accept only one time.
                if(std::find(out.begin(), out.end(), choice) == out.end())
                    out.push_back(choice);
            }
            return out;
        };
        meta->accepts = [lhs, rhs](const std::string &text) {
            return completion_meta_accepts(*lhs, text) || completion_meta_accepts(*rhs, text);
        };
    }
    return meta;
}

/// Append the words of `text` to `words`. With `at_cursor`, the text ends at the cursor, so the last word is added when
/// it is empty or has an open quote. Returns the kept length of the last word.
inline std::size_t split_completion_words(const std::string &text,
                                          const std::string &breaks,
                                          bool at_cursor,
                                          std::vector<std::string> &words) {
    std::string word;
    bool in_word = false;
    char quote = '\0';
    std::size_t quote_mark = 0;
    std::size_t break_mark = 0;
    for(std::size_t pos = 0; pos < text.size(); ++pos) {
        const char ch = text[pos];
        const bool has_next = pos + 1 < text.size();
        if(quote == '\'') {
            if(ch == '\'')
                quote = '\0';
            else
                word += ch;
        } else if(ch == '\\' && has_next &&
                  (quote == '\0' || std::string("\"\\$`").find(text[pos + 1]) != std::string::npos)) {
            in_word = true;
            word += text[++pos];
        } else if(quote != '\0' && ch == quote) {
            quote = '\0';
        } else if(quote == '\0' && (ch == '\'' || ch == '"')) {
            in_word = true;
            quote = ch;
            quote_mark = word.size();
        } else if(quote == '\0' && (ch == ' ' || ch == '\t' || ch == '\n')) {
            if(in_word)
                words.push_back(word);
            word.clear();
            in_word = false;
            break_mark = 0;
        } else {
            in_word = true;
            word += ch;
            if(quote == '\0' && breaks.find(ch) != std::string::npos)
                break_mark = word.size();
        }
    }
    if(in_word || at_cursor)
        words.push_back(word);
    return (quote != '\0') ? quote_mark : break_mark;
}

CLI11_INLINE CompletionLine split_completion_line(const std::string &line,
                                                  std::size_t point,
                                                  const std::string &breaks) {
    CompletionLine out;
    point = (std::min)(point, line.size());
    out.kept = split_completion_words(line.substr(0, point), breaks, true, out.words);
    out.cursor = out.words.size() - 1;
    // The rest of the cursor word is not part of the request. The words after it still count for the positionals.
    const std::string::size_type rest = line.find_first_of(" \t\n", point);
    if(rest != std::string::npos)
        split_completion_words(line.substr(rest), breaks, false, out.words);
    return out;
}

CLI11_INLINE std::string
format_completion_reply_bash(CompletionReply reply, const std::string &word, std::size_t kept) {
    kept = (std::min)(kept, word.size());
    for(CompletionResult &result : reply.results) {
        if(result.value.compare(0, kept, word, 0, kept) == 0)
            result.value.erase(0, kept);
    }
    // A path candidate is a value. Either the prefix or the kept text can be the longer one: `--file=a:b` keeps
    // `--file=a:`, so `a:` comes off each path, and `-fa` keeps nothing, so `-f` goes on.
    const std::size_t named = (std::min)(reply.prefix.size(), word.size());
    const std::string strip = (kept > named) ? word.substr(named, kept - named) : std::string{};
    const std::string lead = (kept < named) ? word.substr(kept, named - kept) : std::string{};
    return format_completion_lines(reply, {{"cur", word.substr(named)}, {"strip", strip}, {"lead", lead}});
}

CLI11_INLINE CompletionDirective completion_hint_directive(CompletionHint hint) {
    switch(hint) {
    case CompletionHint::File:
        return CompletionDirective::Files;
    case CompletionHint::Dir:
        return CompletionDirective::Dirs;
    case CompletionHint::Path:
        return CompletionDirective::Files | CompletionDirective::Dirs;
    case CompletionHint::None:
    default:
        return CompletionDirective::Default;
    }
}

CLI11_INLINE std::shared_ptr<const CompletionMeta> completion_hint_meta(CompletionHint hint) {
    auto meta = std::make_shared<CompletionMeta>();
    meta->hint = hint;
    return meta;
}

CLI11_INLINE std::string escape_completion_field(const std::string &field) {
    std::string out;
    out.reserve(field.size());
    // The backslash is escaped too, so the mapping is reversible. Otherwise a backslash followed by `t` reads as a tab.
    for(char c : field) {
        switch(c) {
        case '\\':
            out += "\\\\";
            break;
        case '\t':
            out += "\\t";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        default:
            out += c;
            break;
        }
    }
    // Must be after the loop. Before it, the loop would escape the added backslash to `\\:`, and the candidate would
    // start with a literal backslash. The unescaper drops the backslash of an unknown sequence.
    if(!out.empty() && out.front() == ':')
        out.insert(out.begin(), '\\');
    return out;
}

CLI11_INLINE std::string completion_script_identifier(const std::string &program_name) {
    std::string out;
    out.reserve(program_name.size());
    for(char c : program_name) {
        const bool usable = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
        out += usable ? c : '_';
    }
    return out;
}

CLI11_INLINE std::string completion_script_bash(const std::string &program_name, const std::string &env_var) {
    static const char *const script = R"bash(#!/usr/bin/env bash

@FUNCTION@_unescape() { # <destination variable> <escaped field>
    local rest=$2 out="" ch
    while [[ -n ${rest} ]]; do
        ch=${rest:0:1}
        rest=${rest:1}
        if [[ ${ch} == "\\" ]]; then
            ch=${rest:0:1}
            rest=${rest:1}
            case ${ch} in
                n) ch=$'\n' ;;
                r) ch=$'\r' ;;
                t) ch=$'\t' ;;
            esac
        fi
        out+=${ch}
    done
    printf -v "$1" '%s' "${out}"
}

@FUNCTION@_describe() { # <length of the longest candidate>
    local longest=$1 tab=$'\t'
    local ci comp desc room i

    for ci in "${!COMPREPLY[@]}"; do
        comp=${COMPREPLY[ci]}
        if [[ ${comp} != *"${tab}"* ]]; then
            continue
        fi
        desc=${comp#*"${tab}"}
        comp=${comp%%"${tab}"*}

        # 4 is the two spaces and two parentheses around the description. COLUMNS is unset in a non-interactive shell.
        # `room` is then negative and the descriptions are dropped.
        room=$(( COLUMNS - longest - 4 ))
        if (( room > 8 )); then
            # Enough room: align the descriptions after the longest candidate.
            for (( i = ${#comp}; i < longest; i++ )); do
                comp+=" "
            done
        else
            # Not enough room to align: start each description directly after its candidate.
            room=$(( COLUMNS - ${#comp} - 4 ))
        fi
        # No room for more than the ellipsis: drop the description.
        if (( room > 3 )); then
            if (( ${#desc} > room )); then
                desc=${desc:0:room-3}
                desc+="..."
            fi
            comp+="  (${desc})"
        fi
        COMPREPLY[ci]=${comp}
    done
}

@FUNCTION@_files() { # [-d]: append the paths that begin with the caller's `cur`, directories only with -d
    # _filedir (bash-completion) reads the word from the caller's local `cur`.
    if declare -F _filedir > /dev/null; then
        _filedir "$@"
        return
    fi
    local kind=-f line
    if [[ ${1-} == -d ]]; then
        kind=-d
    fi
    while IFS='' read -r line; do
        if [[ -n ${line} ]]; then
            COMPREPLY+=("${line}")
        fi
    done <<< "$(compgen "${kind}" -- "${cur}")"
    # Makes readline quote the names and mark the directories. bash 3.2 has no compopt and does neither.
    compopt -o filenames 2>/dev/null
}

@FUNCTION@() {
    local response line comp desc ci tab=$'\t'
    local directive="" cur="" strip="" lead="" longest=0

    # The command word arrives as typed, so apply the expansions the shell applies before it runs the word. A word that
    # could run a command when expanded is not expanded.
    local program=${COMP_WORDS[0]-} unsafe='[$`;&|<>()]'
    if [[ ${program} == *[~\\\"\']* && ! ${program} =~ ${unsafe} && ${program} != *$'\n'* ]]; then
        eval "program=${program}" 2>/dev/null || program=${COMP_WORDS[0]}
    fi
    # Follow an alias that only renames the program, not one that adds words. bash 3.2 has no BASH_ALIASES.
    if (( BASH_VERSINFO[0] >= 4 )) && [[ -n ${program} ]]; then
        local target=${BASH_ALIASES[${program}]-}
        if [[ -n ${target} && ! ${target} =~ [[:space:]] && ! ${target} =~ ${unsafe} ]]; then
            eval "program=${target}" 2>/dev/null || program=${COMP_WORDS[0]}
        fi
    fi

    # The program splits the line itself. COMP_WORDS is not used for that: bash 4 splits it at each break character and
    # bash 3.2 does not. The line goes in the environment, not as arguments. A program that does not answer (older
    # than this script, or with completion off) must not run the line. The one argument is for such a program to
    # reject. With no arguments it could run a default command. MSYS must not convert a line that starts with a path.
    response=$(
        export @ENV@=bash @ENV@_PROTO=@PROTO@ @ENV@_LINE="${COMP_LINE-}" @ENV@_POINT="${COMP_POINT-}"
        export @ENV@_BREAKS="${COMP_WORDBREAKS-}" MSYS2_ENV_CONV_EXCL="@ENV@_${MSYS2_ENV_CONV_EXCL:+;${MSYS2_ENV_CONV_EXCL}}"
        "${program}" --cli11-complete 2>/dev/null
    )

    COMPREPLY=()
    while IFS='' read -r line; do
        # A Windows program ends its lines with CRLF. A carriage return in a field arrives escaped, so one at the end of
        # a line is part of the line ending.
        line=${line%$'\r'}
        if [[ -z ${line} ]]; then
            continue
        elif [[ -n ${directive} ]]; then
            # The directive line is last. A line after it means the reply is not valid.
            COMPREPLY=()
            return 0
        elif [[ ${line} == ":cur="* ]]; then
            # Test these first: any other line that starts with a colon is the directive line.
            @FUNCTION@_unescape cur "${line#:cur=}"
            continue
        elif [[ ${line} == ":strip="* ]]; then
            @FUNCTION@_unescape strip "${line#:strip=}"
            continue
        elif [[ ${line} == ":lead="* ]]; then
            @FUNCTION@_unescape lead "${line#:lead=}"
            continue
        elif [[ ${line} == :* ]]; then
            directive=${line:1}
            continue
        fi

        desc=""
        if [[ ${line} == *"${tab}"* ]]; then
            @FUNCTION@_unescape desc "${line#*"${tab}"}"
        fi
        @FUNCTION@_unescape comp "${line%%"${tab}"*}"
        if (( ${#comp} > longest )); then
            longest=${#comp}
        fi
        if [[ -n ${desc} ]]; then
            COMPREPLY+=("${comp}${tab}${desc}")
        else
            COMPREPLY+=("${comp}")
        fi
    done <<< "${response}"

    # No directive line, or one we cannot read: truncated output or a foreign program
    if [[ ! ${directive} =~ ^[0-9]+$ ]]; then
        COMPREPLY=()
        return 0
    fi

    # The binary could not answer. The usual cause is a script generated for another version.
    if (( (directive & 1) != 0 )); then
        COMPREPLY=()
        return 0
    fi

    # The shell completes paths when the reply asks for them, or when the reply has no candidates and does not forbid
    # paths. No fallback is registered with bash, so a reply that is not valid offers nothing.
    # `cur` is the value typed so far, which _files reads.
    local paths=""
    if (( (directive & 8) != 0 )); then
        paths=any
    elif (( (directive & 16) != 0 )); then
        paths=dirs
    elif (( (directive & 2) == 0 && ${#COMPREPLY[@]} == 0 )); then
        paths=any
    fi
    if [[ -n ${paths} ]]; then
        # Only the paths appended here are values. A positional can send subcommand names with the path hint, and those
        # are full tokens. The fixup starts after the candidates from the binary.
        local from=${#COMPREPLY[@]}
        if [[ ${paths} == dirs ]]; then
            @FUNCTION@_files -d
        else
            @FUNCTION@_files
        fi
        for (( ci = from; ci < ${#COMPREPLY[@]}; ci++ )); do
            COMPREPLY[ci]=${lead}${COMPREPLY[ci]#"${strip}"}
        done
    fi

    # Must be after the paths: the count then includes them, so a single name with paths is listed, not inserted. A
    # description is text in the candidate, so add it only when readline lists the candidates: COMP_TYPE is 9 for a
    # plain TAB and 33, 63 or 64 for the listing variants. Menu completion (37) and insert-completions (42) put each
    # candidate on the line, and bash 3.2 does not report the type. Those cases get the candidates only.
    local listing=""
    case ${COMP_TYPE-} in
        9 | 33 | 63 | 64) listing=1 ;;
    esac
    if [[ -n ${listing} ]] && (( ${#COMPREPLY[@]} > 1 )); then
        @FUNCTION@_describe "${longest}"
    else
        # A single candidate is inserted, so its description would go on the command line.
        for ci in "${!COMPREPLY[@]}"; do
            COMPREPLY[ci]=${COMPREPLY[ci]%%"${tab}"*}
        done
    fi

    if (( (directive & 4) != 0 )); then
        # KeepOrder: stop bash from sorting the candidates.
        compopt -o nosort 2>/dev/null
    fi
    return 0
}

complete -F @FUNCTION@ @PROGRAM@
)bash";

    // Single quotes take everything literally except another single quote
    const std::string quoted = "'" + find_and_replace(program_name, "'", "'\\''") + "'";

    std::string out =
        find_and_replace(script, "@FUNCTION@", "_cli11_complete_" + completion_script_identifier(program_name));
    out = find_and_replace(std::move(out), "@ENV@", env_var);
    out = find_and_replace(std::move(out), "@PROTO@", std::to_string(CLI11_COMPLETE_PROTO_VERSION));
    return find_and_replace(std::move(out), "@PROGRAM@", quoted);
}

}  // namespace detail

// [CLI11:completion_inl_hpp:end]
}  // namespace CLI
