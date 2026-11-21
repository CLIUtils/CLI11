#!/usr/bin/env bash
# Check the generated bash completion script by sourcing it and driving it the way bash does,
# against the example binary passed as $1.
# No set -e: compopt returns non-zero outside a real completion, which the script handles but which
# would abort this harness.
set -u

prog="$1"
# Not `source <(...)`: bash 3.2, which is what macOS ships, reads nothing from the process substitution
eval "$("$prog" --completion bash)"

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

# and a wide one sees them aligned against the longest candidate. 9 is a plain TAB, which lists what it cannot insert.
expect_wide() { # expect_wide <comp_cword> <expected COMPREPLY> <word>...
    local COMP_TYPE=9
    run 80 "$@"
}

# The cursor inside the last word rather than at the end of it. COMP_WORDS holds the whole word either way, so the
# script has to read COMP_LINE and COMP_POINT to find out how much of it is being completed.
expect_at() { # expect_at <characters left after the cursor> <comp_cword> <expected COMPREPLY> <word>...
    local COMP_LINE="${prog} ${*:4}"
    local COMP_POINT=$(( ${#COMP_LINE} - $1 ))
    shift
    run 1 "$@"
}

# The cursor inside a line that is given as text, for a line where the words are not separated by spaces
expect_line() { # expect_line <line after the program> <cursor offset in it> <comp_cword> <expected COMPREPLY> <word>...
    local COMP_LINE="${prog} $1"
    local COMP_POINT=$(( ${#prog} + 1 + $2 ))
    shift 2
    run 1 "$@"
}

# Stand in for the real _filedir, which the bash-completion package supplies to interactive shells only.
# This stub records both of the things the script has to get right: the flag it passes, and `cur`, which the real one
# reads out of its caller's scope to know what to filter on. It appends, as the real one does, since a positional sends
# its subcommand names along with the path hint.
_filedir() { COMPREPLY+=("_filedir[$*|${cur-}]"); }

# A bare word can become a subcommand name or a value for the `files` positional. While a name matches it answers
# alone, since a whole directory listing in the same menu would bury it; once none does, the word is the shell's.
expect 1 "start stop remote" ""
expect 1 "start stop" "st"
expect 1 "start" "sta"
expect 1 "_filedir[|zzz]" "zzz"
# The cursor is on the program name itself
expect 0 "" ""

# Inside a subcommand, and reached by an alias
expect 2 "add remove rm sync" "remote" ""
expect 2 "remove rm" "remote" "r"
expect 3 "" "remote" "add" ""

# Option names, from whichever subcommand the walk ended in. `--config` and `--workdir` live in an option group, which
# a parse reaches through, so they are offered here too — after the app's own, which is the order a parse tries them in.
expect 1 "--help --completion --verbose --level --image --tag --config --workdir" "--"
expect 1 "--config" "--conf"
expect 1 "--verbose" "--v"
expect 1 "-v" "-v"
expect 2 "--help --force" "remote" "--"

# The values an option is checked against, in the order they were declared
expect 2 "fast slow" "--level" ""
expect 2 "slow" "--level" "s"
expect 2 "fast slow" "-l" ""
# A flag consumes nothing, so what follows it is a token of its own
expect 2 "start stop remote" "--verbose" ""

# An option that takes any number of words goes on offering its values, and once past the one it must have, the word is
# also one of the names that would stop it
expect 2 "alpha beta" "--tag" ""
expect 3 "start stop remote alpha beta" "--tag" "alpha" ""
# A word it would stop at is not its value: a dash-shaped one is an option name again
expect 3 "--verbose" "--tag" "alpha" "--v"

# A path validator says what kind of path it wants and offers nothing, leaving the walking of it to the shell
expect 2 "_filedir[|]" "--config" ""
expect 2 "_filedir[-d|]" "--workdir" ""
# A partial value is filtered by _filedir, which only sees it because the script puts it in `cur` first
expect 2 "_filedir[|/et]" "--config" "/et"

# Bash tore the line apart at every COMP_WORDBREAKS character, so the script puts the words back together and asks the
# binary about `--level=f`. Readline replaces only the piece after the `=`, so that much of each whole-token candidate
# is what is left here.
expect 3 "fast" "--level" "=" "f"
expect 2 "fast slow" "--level" "="
# A path after an `=` is still the shell's to walk, and only the value goes to _filedir
expect 3 "_filedir[|/et]" "--config" "=" "/et"
# The cursor on the `=` is a cursor on the empty value after it, not on a path beginning with an `=`
expect 2 "_filedir[|]" "--config" "="
# A flag has no value to complete, so nothing is offered: not the subcommands, and not a file either
expect 3 "" "--verbose" "=" "v"
# and a space after the `=` does not finish the value: an empty one is no value at all, so the option still takes the
# next word, and `completion --level= fast` really does parse
expect 3 "fast slow" "--level" "=" ""

# A short name carries its value in the same word, and that whole word is what readline replaces
expect 1 "-lslow" "-ls"
# and the flags in front of it are part of that word too
expect 1 "-vlfast" "-vlf"
expect 1 "-vl" "-vl"
# and the value may be the next word instead, in which case it belongs to the last name in the bundle
expect 2 "fast slow" "-vl" ""

# With the cursor in the middle of a word, only what is in front of it is being completed; the rest is text readline
# will leave on the line, and answering for it would have the answer inserted in front of what is already there
expect_at 1 1 "-v" "-vi"
expect_at 4 1 "--verbose" "--verbose"
expect_at 0 1 "--verbose" "--verbose"
# The cursor in the name of `--level=f`: the `=` and the value after the cursor are not part of the word
expect_line "--level=f" 4 1 "--level" "--level" "=" "f"
expect_line "--level=f" 7 1 "--level" "--level" "=" "f"
# bash 3.2 does not split COMP_WORDS at a break character, but its readline still replaces only the text after one
expect 1 "fast" "--level=f"
expect 1 "3.19 3.20" "--image=alpine:"
expect 1 "fast" '--level="fa'
expect_line "--level=f" 4 1 "--level" "--level=f"
expect_line "--level=f" 7 1 "--level" "--level=f"
# A path written onto a short name keeps it: the shell never saw the `-c`, so the script puts it back on
expect 1 "-c_filedir[|/et]" "-c/et"

# A colon is a word break as well. _filedir gets the whole value rather than the piece after the last colon, and its
# answers come back trimmed to that piece, which is all readline will replace.
expect 4 "_filedir[|a:b]" "--config" "a" ":" "b"
expect 5 "_filedir[|a:b]" "--config" "=" "a" ":" "b"
# A colon inside a candidate is trimmed the same way, so `alpine:` is not inserted a second time
expect 4 "3.19 3.20" "--image" "=" "alpine" ":"
# and a value written onto a short name is trimmed against the same boundary
expect 2 "3.19 3.20" "-ialpine" ":"

# Words arrive as typed, so the script removes the quotes and the escapes before the binary reads them
expect 2 "add remove rm sync" "'remote'" ""
expect 2 "add remove rm sync" '"remote"' ""
expect 2 "add remove rm sync" 'rem\ote' ""
# Readline replaces the text after an open quote, so the candidates have no quote
expect 2 "fast" "--level" '"fa'
expect 2 "fast" "--level" "'fa"
expect 3 "fast" "--level" "=" '"fa'
# and a break character in the quotes is not a break, so the candidates keep the text in front of it
expect 2 "alpine:3.19 alpine:3.20" "--image" '"alpine:'
expect 3 "alpine:3.19 alpine:3.20" "--image" "=" '"alpine:'
expect 2 "_filedir[|my fi]" "--config" 'my\ fi'
expect 2 "_filedir[|my fi]" "--config" '"my fi'

# Past the end-of-options marker every word is a value for the positional, so neither an option name nor a subcommand
# name is a candidate, but the positional's own hint still is, dash-shaped word or not.
expect 2 "_filedir[|--v]" "--" "--v"
expect 2 "_filedir[|st]" "--" "st"
# and the marker holds for the rest of the line, through a word that would otherwise move the walk
expect 3 "_filedir[|--f]" "--" "start" "--f"

# `++` closes the subcommand it appears in, so the candidates come from the parent again
expect 2 "" "remote" "--v"
expect 3 "--verbose" "remote" "++" "--v"
# The limit of one subcommand is reached, so a parse reads the next word as a file and not as a name
expect 3 "_filedir[|]" "remote" "++" ""
# The marker outranks it: past `--` a `++` is a positional value like any other word
expect 4 "" "remote" "--" "++" "--f"

# Descriptions, shown beside the candidates and aligned against the longest of them
expect_wide 1 "start   (Get going) stop    (Do you really want to stop?) remote  (Work with remotes)" ""
expect_wide 2 "--help   (Print this help message and exit) --force  (Do it anyway)" "remote" "--"
# A lone candidate is inserted rather than listed, so it loses its description
expect_wide 1 "start" "sta"
# and one that does not fit the terminal is cut short
COMP_TYPE=9 run 30 1 "start  (Get going) stop   (Do you really want...)" "st"
unset COMP_TYPE
# Menu completion puts each candidate on the line in turn, so a description would be typed into the command. bash 3.2
# does not say which kind of completion it is doing, and gets the same.
COMP_TYPE=37 run 80 1 "start stop remote" ""
COMP_TYPE=42 run 80 1 "start stop remote" ""
unset COMP_TYPE
run 80 1 "start stop remote" ""
# The two that list without inserting keep them
COMP_TYPE=63 run 80 1 "start  (Get going) stop   (Do you really want to stop?)" "st"
unset COMP_TYPE
# A tab in a description reaches the binary's output escaped, so the script has to put it back
expect_wide 2 $'add     (Add a remote) remove  (Drop a remote) rm      (Drop a remote) sync    (Push\tand pull)' \
    "remote" ""

# A user can take `:` out of COMP_WORDBREAKS. Readline then replaces the whole word, so the candidates stay whole.
saved_breaks=${COMP_WORDBREAKS}
COMP_WORDBREAKS=${COMP_WORDBREAKS//:/}
expect 2 "alpine:3.19 alpine:3.20" "--image" "alpine:"
expect 3 "alpine:3.19 alpine:3.20" "--image" "=" "alpine:"
COMP_WORDBREAKS=${COMP_WORDBREAKS//=/}
expect 1 "--level=fast" "--level=f"
expect 1 "--image=alpine:3.19 --image=alpine:3.20" "--image=alpine:"
COMP_WORDBREAKS=${saved_breaks}

# The command word arrives as typed, so the script has to expand it as the shell would before it can run it
real=${prog}
saved_home=${HOME-}
HOME=$(dirname "${real}")
prog="~/$(basename "${real}")"
expect 1 "start" "sta"
HOME=${saved_home}
prog="\"${real}\""
expect 1 "start" "sta"
prog="'${real}'"
expect 1 "start" "sta"
prog=${real/completion/comp\\letion}
expect 1 "start" "sta"
# but a word that would run a command is not expanded
canary="${TMPDIR:-/tmp}/cli11_completion_canary_$$"
prog="\$(touch ${canary})"
expect 1 "" "sta"
if [[ -e ${canary} ]]; then
    rm -f "${canary}"
    echo "FAIL: the command word was executed during expansion" >&2
    exit 1
fi
# An alias that only renames the program is followed. bash 3.2 has no BASH_ALIASES to read it from.
if (( BASH_VERSINFO[0] >= 4 )); then
    shopt -s expand_aliases
    alias cli11_completion_alias="${real}"
    prog=cli11_completion_alias
    expect 1 "start" "sta"
    alias cli11_completion_alias="${real} --verbose"
    expect 1 "" "sta"
    unalias cli11_completion_alias
fi
prog=${real}

# A script that outlived the binary it was generated against asks for a version the binary does not know, and is
# answered with the Error directive alone
reply=$(CLI11_COMPLETE=bash CLI11_COMPLETE_INDEX=1 CLI11_COMPLETE_PROTO=99 \
    CLI11_COMPLETE_WORDS=$'\x1f' "$prog" --cli11-complete)
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

# and it must not be handed the line that is being typed: a program that does not answer requests would run it. The
# words travel in the environment, and the one argument is a fixed one.
probe_file="${TMPDIR:-/tmp}/cli11_completion_probe_$$"
probe() { printf '%s\n' "$#" "$@" "${CLI11_COMPLETE_WORDS-}" > "${probe_file}"; }
prog=probe
expect 2 "" "remote" "--force; x"
probe_got=$(cat "${probe_file}")
rm -f "${probe_file}"
probe_want=$'1\n--cli11-complete\nremote\x1f--force; x\x1f'
if [[ ${probe_got} != "${probe_want}" ]]; then
    echo "FAIL: the program was run with [${probe_got}] want [${probe_want}]" >&2
    exit 1
fi
echo "ok: the line is not in the arguments"

# The directives of a reply, apart from any program. Either kind of path is completed like a file, and a reply that
# knows nothing about the value gets paths when it has no candidates of its own.
reply_path() { printf ':24\n'; }
prog=reply_path
expect 1 "_filedir[|./fi]" "./fi"
reply_unknown() { printf ':prefix=-p\n:0\n'; }
prog=reply_unknown
# The value is what the paths are looked up by, and the name it is written onto goes back on the answers
expect 1 "-p_filedir[|./fi]" "-p./fi"
reply_named() { printf 'start\n:0\n'; }
prog=reply_named
expect 1 "start" "st"
reply_none() { printf ':2\n'; }
prog=reply_none
expect 1 "" "st"
prog=${real}

# Nothing is registered for bash to fall back on when the function offers nothing, which is what keeps a reply that
# cannot be used from turning into a list of files
registered=$(complete -p completion)
if [[ ${registered} != "complete -F _cli11_complete_completion completion" ]]; then
    echo "FAIL: registered as [${registered}]" >&2
    exit 1
fi
echo "ok: ${registered}"

# Without the bash-completion package the script finds the paths itself
unset -f _filedir
scratch=$(mktemp -d "${TMPDIR:-/tmp}/cli11_completion_XXXXXX")
touch "${scratch}/alpha.txt" "${scratch}/a star*.txt"
mkdir "${scratch}/bdir"
prog="$(cd "$(dirname "${real}")" && pwd)/$(basename "${real}")"
pushd "${scratch}" > /dev/null || exit 1
expect 2 "alpha.txt" "--config" "al"
expect 2 "bdir" "--workdir" ""
expect 1 "-calpha.txt" "-cal"
expect 3 "alpha.txt" "--config" "=" "al"
# A name is taken as it is, whatever is in it
expect 2 "a star*.txt" "--config" "a "
expect 2 "a star*.txt" "--config" 'a\ '
expect 2 "a star*.txt" "--config" '"a '
popd > /dev/null || exit 1
rm -rf "${scratch}"
prog=${real}

echo "all ok"
