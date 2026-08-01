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
        out += result.value;
        out += '\n';
    }
    out += ':';
    out += std::to_string(static_cast<int>(reply.directive));
    out += '\n';
    return out;
}

namespace detail {

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

@FUNCTION@() {
    local response line
    local directive=""

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
        else
            COMPREPLY+=("${line}")
        fi
    done <<< "${response}"

    # No directive line, or one we cannot read: truncated output or a foreign program
    if [[ ! ${directive} =~ ^[0-9]+$ ]]; then
        COMPREPLY=()
        return 0
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
