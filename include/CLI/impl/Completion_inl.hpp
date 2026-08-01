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
#include <cstdint>
#include <string>
#include <vector>
// [CLI11:public_includes:end]

#include "../StringTools.hpp"

namespace CLI {
// [CLI11:completion_inl_hpp:verbatim]

CLI11_INLINE std::string format_completion_reply(const CompletionReply &reply) {
    std::string out;
    for(const CompletionResult &result : reply.results) {
        out += detail::escape_completion_field(result.value);

        // A description is one line by definition -- the shell has one line to show it on -- so anything the user put
        // after the first break is cut rather than escaped into a candidate the listing cannot render.
        const std::string description = result.description.substr(0, result.description.find_first_of("\r\n"));
        if(!description.empty()) {
            out += '\t';
            out += detail::escape_completion_field(description);
        }
        out += '\n';
    }
    out += ':';
    out += std::to_string(static_cast<int>(reply.directive));
    out += '\n';
    return out;
}

namespace detail {

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
    local response line comp desc tab=$'\t'
    local directive="" longest=0

    response=$(@ENV@=bash @ENV@_INDEX="${COMP_CWORD}" "${COMP_WORDS[@]}" 2>/dev/null)

    COMPREPLY=()
    while IFS='' read -r line; do
        if [[ -z ${line} ]]; then
            continue
        elif [[ -n ${directive} ]]; then
            # The directive line is last, so anything after it is not a reply we understand
            COMPREPLY=()
            return 0
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

    if (( ${#COMPREPLY[@]} == 1 )); then
        # A lone candidate is inserted rather than listed, so its description would land on the command line
        COMPREPLY[0]=${COMPREPLY[0]%%"${tab}"*}
    else
        @FUNCTION@_describe "${longest}"
    fi

    if (( (directive & 2) != 0 )); then
        compopt +o default 2>/dev/null
    fi
    return 0
}

complete -o default -F @FUNCTION@ @PROGRAM@
)bash";

    std::string out =
        find_and_replace(script, "@FUNCTION@", "_cli11_complete_" + completion_script_identifier(program_name));
    out = find_and_replace(std::move(out), "@ENV@", env_var);
    return find_and_replace(std::move(out), "@PROGRAM@", program_name);
}

}  // namespace detail

// [CLI11:completion_inl_hpp:end]
}  // namespace CLI
