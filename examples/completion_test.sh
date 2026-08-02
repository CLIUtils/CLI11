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

run() { # run <terminal width> <comp_cword> <expected COMPREPLY> <word>...
    local COLUMNS="$1" cword="$2" want="$3"
    shift 3
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

# A terminal one column wide has no room for a description, so these see the bare candidates
expect() { # expect <comp_cword> <expected COMPREPLY> <word>...
    run 1 "$@"
}

# and a wide one sees them aligned against the longest candidate
expect_wide() { # expect_wide <comp_cword> <expected COMPREPLY> <word>...
    run 80 "$@"
}

# Stand in for the real _filedir, which the bash-completion package supplies to interactive shells only. This
# stub records both of the things the script has to get right: the flag it passes, and `cur`, which the real
# one reads out of its caller's scope to know what to filter on.
_filedir() { COMPREPLY=("_filedir[$*|${cur-}]"); }

expect 1 "start stop remote" ""
expect 1 "start stop" "st"
expect 1 "start" "sta"
expect 1 "" "zzz"
# The cursor is on the program name itself
expect 0 "" ""

# Inside a subcommand, and reached by an alias
expect 2 "add remove rm sync" "remote" ""
expect 2 "remove rm" "remote" "r"
expect 3 "" "remote" "add" ""

# Option names, from whichever subcommand the walk ended in
expect 1 "--help --completion --verbose --level --config --workdir" "--"
expect 1 "--verbose" "--v"
expect 1 "-v" "-v"
expect 2 "--help --force" "remote" "--"

# The values an option is checked against, in the order they were declared
expect 2 "fast slow" "--level" ""
expect 2 "slow" "--level" "s"
expect 2 "fast slow" "-l" ""
# A flag consumes nothing, so what follows it is a token of its own
expect 2 "start stop remote" "--verbose" ""

# A path validator says what kind of path it wants and offers nothing, leaving the walking of it to the shell
expect 2 "_filedir[|]" "--config" ""
expect 2 "_filedir[-d|]" "--workdir" ""
# A partial value is filtered by _filedir, which only sees it because the script puts it in `cur` first
expect 2 "_filedir[|/et]" "--config" "/et"

# Bash splits on COMP_WORDBREAKS, so this reaches the binary as three words and the cursor sits on the value
expect 3 "" "--force" "=" ""

# Descriptions, shown beside the candidates and aligned against the longest of them
expect_wide 1 "start   (Get going) stop    (Do you really want to stop?) remote  (Work with remotes)" ""
expect_wide 2 "--help   (Print this help message and exit) --force  (Do it anyway)" "remote" "--"
# A lone candidate is inserted rather than listed, so it loses its description
expect_wide 1 "start" "sta"
# and one that does not fit the terminal is cut short
run 30 1 "start  (Get going) stop   (Do you really want...)" "st"
# A tab in a description reaches the binary's output escaped, so the script has to put it back
expect_wide 2 $'add     (Add a remote) remove  (Drop a remote) rm      (Drop a remote) sync    (Push\tand pull)' \
    "remote" ""

# A script that outlived the binary it was generated against asks for a version the binary does not know, and is
# answered with the Error directive alone
reply=$(CLI11_COMPLETE=bash CLI11_COMPLETE_INDEX=1 CLI11_COMPLETE_PROTO=99 "$prog" "")
if [[ ${reply} != ":1" ]]; then
    echo "FAIL: a foreign protocol version got [${reply}] want [:1]" >&2
    exit 1
fi
echo "ok: protocol version 99 -> [${reply}]"

# and on this side of it, candidates arriving with that directive are dropped rather than offered
error_reply() { printf 'start\n:1\n'; }
prog=error_reply
expect 1 "" "st"

# A program that is not a CLI11 app must not have its output mistaken for a reply
prog=/bin/echo
expect 1 "" "st"

echo "all ok"
