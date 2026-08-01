#!/usr/bin/env bash
# Check the generated bash completion script by sourcing it and driving it the way bash does,
# against the example binary passed as $1.
# No set -e: compopt returns non-zero outside a real completion, which the script handles but which
# would abort this harness.
set -u

prog="$1"
source <("$prog" --completion bash)

if ! declare -F _cli11_complete_completion > /dev/null; then
    echo "FAIL: the generated script did not define the completion function" >&2
    exit 1
fi

expect() { # expect <comp_cword> <expected COMPREPLY> <word>...
    local cword="$1" want="$2"
    shift 2
    COMP_WORDS=("$prog" "$@")
    COMP_CWORD="$cword"
    COMPREPLY=()
    _cli11_complete_completion
    local got="${COMPREPLY[*]-}"
    if [[ ${got} != "${want}" ]]; then
        echo "FAIL: cword=${cword} words=[$*] got [${got}] want [${want}]" >&2
        exit 1
    fi
    echo "ok: cword=${cword} [$*] -> [${got}]"
}

expect 1 "start stop remote" ""
expect 1 "start stop" "st"
expect 1 "start" "sta"
expect 1 "" "zzz"
# The cursor is on the program name itself
expect 0 "" ""

# Inside a subcommand, and reached by an alias
expect 2 "add remove rm" "remote" ""
expect 2 "remove rm" "remote" "r"
expect 3 "" "remote" "add" ""

# Option names, from whichever subcommand the walk ended in
expect 1 "--help --completion --verbose" "--"
expect 1 "--verbose" "--v"
expect 1 "-v" "-v"
expect 2 "--help --force" "remote" "--"

# Bash splits on COMP_WORDBREAKS, so this reaches the binary as three words and the cursor sits on the value
expect 3 "" "--force" "=" ""

# A program that is not a CLI11 app must not have its output mistaken for a reply
prog=/bin/echo
expect 1 "" "st"

echo "all ok"
