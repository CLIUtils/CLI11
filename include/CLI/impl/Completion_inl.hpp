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
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
// [CLI11:public_includes:end]

#include "../StringTools.hpp"

namespace CLI {
// [CLI11:completion_inl_hpp:verbatim]

CLI11_INLINE std::string format_completion_reply(const CompletionReply &reply) {
    std::string out;
    for(const CompletionResult &result : reply.results) {
        out += detail::escape_completion_field(result.value);

        // A description is one line by definition, the shell having one line to show it on, so anything the user put
        // after the first break is cut rather than escaped into a candidate the listing cannot render.
        const std::string description = result.description.substr(0, result.description.find_first_of("\r\n"));
        if(!description.empty()) {
            out += '\t';
            out += detail::escape_completion_field(description);
        }
        out += '\n';
    }
    // Ahead of the directive line, which stays last so a script can tell a complete reply from a truncated one
    if(!reply.prefix.empty()) {
        out += ":prefix=";
        out += detail::escape_completion_field(reply.prefix);
        out += '\n';
    }
    out += ':';
    out += std::to_string(static_cast<int>(reply.directive));
    out += '\n';
    return out;
}

namespace detail {

CLI11_INLINE std::shared_ptr<const CompletionMeta>
intersect_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                          const std::shared_ptr<const CompletionMeta> &rhs) {
    if(!lhs)
        return rhs;
    if(!rhs)
        return lhs;

    auto meta = std::make_shared<CompletionMeta>();
    // There is no intersecting "a file" with "a directory", so the left one wins. `ExistingFile & size_check`
    // should still complete files, and that is the way round the combination is usually written
    meta->hint = (lhs->hint != CompletionHint::None) ? lhs->hint : rhs->hint;
    if(!lhs->choices || !rhs->choices) {
        meta->choices = lhs->choices ? lhs->choices : rhs->choices;
        return meta;
    }

    meta->choices = [lhs, rhs]() {
        // Walking the left one keeps its order, which is the order a declared set is offered in
        const std::vector<std::string> allowed = rhs->choices();
        std::vector<std::string> out;
        for(const std::string &choice : lhs->choices()) {
            if(std::find(allowed.begin(), allowed.end(), choice) != allowed.end())
                out.push_back(choice);
        }
        return out;
    };
    return meta;
}

CLI11_INLINE std::shared_ptr<const CompletionMeta>
unite_completion_meta(const std::shared_ptr<const CompletionMeta> &lhs,
                      const std::shared_ptr<const CompletionMeta> &rhs) {
    if(!lhs)
        return rhs;
    if(!rhs)
        return lhs;

    auto meta = std::make_shared<CompletionMeta>();
    // A hint stands for a set of values too large to write down, so a union with one is still that hint
    meta->hint = (lhs->hint != CompletionHint::None) ? lhs->hint : rhs->hint;
    if(!lhs->choices || !rhs->choices) {
        meta->choices = lhs->choices ? lhs->choices : rhs->choices;
        return meta;
    }

    meta->choices = [lhs, rhs]() {
        std::vector<std::string> out = lhs->choices();
        for(const std::string &choice : rhs->choices()) {
            // A value both sides accept is one value, and offering it twice would read as two ways to say it
            if(std::find(out.begin(), out.end(), choice) == out.end())
                out.push_back(choice);
        }
        return out;
    };
    return meta;
}

CLI11_INLINE std::string escape_completion_field(const std::string &field) {
    std::string out;
    out.reserve(field.size());
    // The backslash is escaped along with the characters that need it, or the mapping would not be reversible: a
    // field holding a literal backslash followed by a `t` would arrive indistinguishable from one holding a tab.
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
    // After the loop, not before: escaping the backslash first would turn it into `\\:` and hand back a candidate
    // that begins with a literal backslash. The unescaper drops the backslash of any sequence it does not know.
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

        # Two spaces and two parentheses sit between the candidate and its description. COLUMNS is unset in a
        # non-interactive shell, which makes this negative and drops the descriptions rather than wrapping them.
        room=$(( COLUMNS - longest - 4 ))
        if (( room > 8 )); then
            # There is room to align every description against the longest candidate
            for (( i = ${#comp}; i < longest; i++ )); do
                comp+=" "
            done
        else
            # Aligning would leave too little for the text, so let each description start right after its candidate
            room=$(( COLUMNS - ${#comp} - 4 ))
        fi
        # Below the width of the ellipsis there is nothing left to say, so the description is dropped instead
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

@FUNCTION@() {
    local response line comp desc ci ii tab=$'\t'
    local directive="" prefix="" longest=0

    # Readline completes the text in front of the cursor and leaves the rest of the word on the line, but COMP_WORDS
    # holds whole words. Sending that tail along would have the binary answer for a word nobody typed, and readline
    # then insert the answer in front of the tail.
    local -a raw=("${COMP_WORDS[@]}")
    local comp_line=${COMP_LINE-} point=${COMP_POINT-0}
    local after=${comp_line:point}
    after=${after%%[[:space:]]*}
    # Only when it really is the end of that word: with the cursor on the `=` of `--file=x`, the `x` after it belongs
    # to the next word of COMP_WORDS rather than to this one.
    if [[ -n ${after} && ${raw[COMP_CWORD]-} == *"${after}" ]]; then
        raw[COMP_CWORD]=${raw[COMP_CWORD]%"${after}"}
    fi

    # Readline splits the line on COMP_WORDBREAKS, whose default holds `=` and `:`, before bash hands it over, so
    # `--file=/et` arrives as the three words `--file`, `=`, `/et`. Rejoin them into the arguments the program would
    # have received, and move the cursor index with them. Spaces around a break character are not recoverable, so
    # `a : b` reads as `a:b`; an empty word is what bash sends for a cursor past a space, so it still starts a word.
    local -a words=()
    local cword=0 count=0 word glue="" is_break
    for (( ii = 0; ii < ${#raw[@]}; ii++ )); do
        word=${raw[ii]}
        is_break=""
        if [[ -n ${word} && -z ${word//[=:]/} ]]; then
            is_break=1
        fi
        # Never into word 0: gluing onto the program name would invoke something that does not exist
        if [[ -n ${word} && -n ${glue}${is_break} ]] && (( count > 1 )); then
            words[count-1]+=${word}
        else
            words+=("${word}")
            (( ++count ))
        fi
        if (( ii == COMP_CWORD )); then
            cword=$(( count - 1 ))
        fi
        glue=${is_break}
    done

    # Readline puts back only the part of the word that follows the last break character in it, so a whole-token
    # candidate has to be trimmed down to that much before it is handed over.
    word=${words[cword]-}
    local head=${word%"${word##*[=:]}"}

    response=$(@ENV@=bash @ENV@_INDEX="${cword}" @ENV@_PROTO=@PROTO@ "${words[@]}" 2>/dev/null)

    COMPREPLY=()
    while IFS='' read -r line; do
        # A program built for Windows writes its lines with a CRLF. A carriage return inside a field arrives escaped,
        # so one at the end of a line can only be part of the line ending.
        line=${line%$'\r'}
        if [[ -z ${line} ]]; then
            continue
        elif [[ -n ${directive} ]]; then
            # The directive line is last, so anything after it is not a reply we understand
            COMPREPLY=()
            return 0
        elif [[ ${line} == ":prefix="* ]]; then
            # Tested first, since the directive line is any other line beginning with a colon
            @FUNCTION@_unescape prefix "${line#:prefix=}"
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
        comp=${comp#"${head}"}
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

    # The binary could not answer the request, most likely because this script was generated against another version
    if (( (directive & 1) != 0 )); then
        COMPREPLY=()
        return 0
    fi

    # A path is the shell's to complete. _filedir comes from the bash-completion package; without it the `-o default`
    # on the complete line leaves readline to it. It reads the word from a caller's local named `cur` rather than from
    # an argument, which dynamic scoping is what makes work.
    #
    # Its answers are whole values, while readline replaces the word from its last break character on, so the two
    # boundaries have to be lined up, and either can be the later one. `--file=a:b` has the head of the value on the
    # line already, so it comes back off the candidates; `-fa` has no break at all, so the `-f` goes on.
    local cur=${word#"${prefix}"} value_head="" value_lead=""
    if (( ${#head} >= ${#prefix} )); then
        value_head=${head#"${prefix}"}
    else
        value_lead=${prefix#"${head}"}
    fi
    if (( (directive & 24) != 0 )); then
        # Only what _filedir appends is in value coordinates. A positional sends its subcommand names along with the
        # path hint, and those are whole tokens already, so the fixup starts where the candidates from the binary end.
        local from=${#COMPREPLY[@]}
        if (( (directive & 16) != 0 )); then
            if declare -F _filedir > /dev/null; then
                _filedir -d
            else
                compopt +o default -o dirnames 2>/dev/null
            fi
        elif declare -F _filedir > /dev/null; then
            _filedir
        else
            compopt -o default 2>/dev/null
        fi
        for (( ci = from; ci < ${#COMPREPLY[@]}; ci++ )); do
            COMPREPLY[ci]=${value_lead}${COMPREPLY[ci]#"${value_head}"}
        done
    fi

    # After the paths, so that a name sharing the reply with them is still listed rather than inserted
    if (( ${#COMPREPLY[@]} == 1 )); then
        # A lone candidate is inserted rather than listed, so its description would land on the command line
        COMPREPLY[0]=${COMPREPLY[0]%%"${tab}"*}
    else
        @FUNCTION@_describe "${longest}"
    fi

    if (( (directive & 2) != 0 )); then
        compopt +o default 2>/dev/null
    fi
    if (( (directive & 4) != 0 )); then
        # The candidates came back in a meaningful order, which bash would otherwise sort away
        compopt -o nosort 2>/dev/null
    fi
    return 0
}

complete -o default -F @FUNCTION@ @PROGRAM@
)bash";

    std::string out =
        find_and_replace(script, "@FUNCTION@", "_cli11_complete_" + completion_script_identifier(program_name));
    out = find_and_replace(std::move(out), "@ENV@", env_var);
    out = find_and_replace(std::move(out), "@PROTO@", std::to_string(CLI11_COMPLETE_PROTO_VERSION));
    return find_and_replace(std::move(out), "@PROGRAM@", program_name);
}

}  // namespace detail

// [CLI11:completion_inl_hpp:end]
}  // namespace CLI
