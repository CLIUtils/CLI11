# Shell completion {#book-completion}

CLI11 programs can complete their own command lines. A small script per shell
asks the program what may follow the word being typed, so the answers are the
program's real subcommands, options and values, and they stay right as the
program changes. Only bash is supported so far.

Answering those requests is built in and on by default; what a user has to
install is the script.

## Printing the script

The usual way to hand out the script is a flag, which is off until you ask for
it:

```cpp
CLI::App app{"A program that knows how to complete itself", "myprog"};
app.set_completion_flag();  // --completion SHELL
```

Then, in a shell:

```bash
source <(myprog --completion bash)
```

Put that line in `~/.bashrc` for a program you have built yourself, or write the
output to a file under `/usr/share/bash-completion/completions/` when packaging
one. A packaged file is safe to ship: the script carries the version of the
reply format it was generated for, and a binary that does not recognise the
number offers nothing rather than something wrong.

`set_completion_flag` takes the flag name and the help text, and an empty name
removes the flag again, exactly as `set_version_flag` does. Without a flag the
script is still available from `app.get_completion_script("bash")`, which throws
a `CLI::ValidationError` for a shell it cannot generate.

## What is completed

No extra code is needed for any of this:

- Subcommand names, their aliases, and everything inside them, however deeply
  nested.
- Option names, with their descriptions shown beside them, in every spelling the
  program accepts: long, short, and `/flag` where `allow_windows_style_options`
  is on. Descriptions need bash 4 or later, and are left out for menu
  completion, which would type them into the command.
- The values of an option checked against `CLI::IsMember`, and the keys of a
  `CLI::Transformer` or `CLI::CheckedTransformer`, in the order they were
  declared.
- File and directory names for an option checked against `CLI::ExistingFile`,
  `CLI::ExistingDirectory` or `CLI::ExistingPath` — the shell completes those
  itself, since it is the one that knows the directory the line is being typed
  in. An option that nothing is known about gets the same.
- Positionals, from the same two sources.

A validator given an `application_index` applies to the value at that index
only, so each value of a pair can have its own list.

Names are matched the way the parser matches them, so `ignore_case` and
`ignore_underscore` apply, and so do the filters given to `CLI::IsMember` and
the transformers. Nameless option groups and nameless subcommands are searched
too, because a parse reaches through them, and a `fallthrough` subcommand also
offers the options of its parent. An option or subcommand hidden from help with
an empty group is not offered.

## Values the validators do not describe

An option whose values are not enumerated by a validator can say what shape they
have:

```cpp
std::vector<std::string> files;
app.add_option("files", files, "What to work on")
    ->completion_hint(CLI::CompletionHint::File);
```

The hints are `File`, `Dir`, `Path` and `None`. This is the way to complete
paths for an option that accepts a name which does not exist yet, where
`ExistingFile` would reject the value the user is typing. A declared hint
replaces what the validators say rather than adding to it.

## Turning it off, and the environment

A request arrives in the environment, so completion adds no flag and no
subcommand and changes no help output. Two settings control it:

```cpp
app.disable_completion();                       // ignore requests entirely
app.set_completion_env_var("MYPROG_COMPLETE");  // the default is CLI11_COMPLETE
```

The generated script uses whichever variable is configured, so the two stay
together; generate a new script after changing it. The name goes into the script
as a shell variable, so it has to be a shell identifier, and
`set_completion_env_var` throws `CLI::IncorrectConstruction` for one that is
not.

## How it works

On TAB, the script runs the program again with the request in the environment.
Completing `myprog st` runs:

```bash
CLI11_COMPLETE=bash CLI11_COMPLETE_INDEX=1 CLI11_COMPLETE_PROTO=1 \
  CLI11_COMPLETE_WORDS=1 CLI11_COMPLETE_WORD_1=st myprog --cli11-complete
```

| Variable                  | Example | Meaning                                                                                                  |
| ------------------------- | ------- | -------------------------------------------------------------------------------------------------------- |
| `CLI11_COMPLETE`          | `bash`  | The shell to answer for. Setting it is what makes the run a completion request rather than a normal one. |
| `CLI11_COMPLETE_INDEX`    | `1`     | Which word the cursor is in, counting the program name as word 0.                                        |
| `CLI11_COMPLETE_PROTO`    | `1`     | The version of the reply format the script was generated for.                                            |
| `CLI11_COMPLETE_WORDS`    | `1`     | How many words the line has after the program name.                                                      |
| `CLI11_COMPLETE_WORD_<n>` | `st`    | Word `n` of the line, numbered as `CLI11_COMPLETE_INDEX` numbers them.                                   |

All of them are named after the first, so `set_completion_env_var` renames them
together. The program removes them from the environment before it does anything
else, so nothing it runs later inherits the request.

The words of the line are never the arguments of the run. A script can outlive
the program it was generated for, and a program that does not answer requests
(an older build, or one with `disable_completion`) runs what it is given; if
that were the line, TAB would run the command being typed. The one argument is
fixed and is not read. It is there for such a program to reject, as a CLI11
program does with an option it does not know.

The program then classifies those words the way a parse would — which subcommand
the line has reached, which option is collecting values, which positional's turn
it is — and prints what it would accept next:

```text
start<TAB>Get going
stop<TAB>Do you really want to stop?
:2
```

The candidates come first, one per line, each followed by a tab and its
description where it has one. A backslash, a tab or a line break inside either
field is escaped, so a line always means one candidate.

An optional `:prefix=` line comes next, carrying the part of the word that is
already on the line and is not the value — the `--config=` of `--config=/et`.
The shell needs it because it replaces less than a whole word.

The last line is always the directive, a bit mask telling the shell what else it
may do. The `2` above says the word being completed is not a file name, so bash
is not to add any:

| Bit | Name            | Meaning                                                         |
| --- | --------------- | --------------------------------------------------------------- |
| 1   | `Error`         | The request could not be answered; ignore the whole reply.      |
| 2   | `NoFileComp`    | Do not add file names of your own.                              |
| 4   | `KeepOrder`     | The candidates are in a meaningful order; do not sort them.     |
| 8   | `FilterFileExt` | The value is a file name; complete it.                          |
| 16  | `FilterDirs`    | The value is a directory name; complete it. With 8, either one. |

With none of these set, nothing is known about the word, and the script offers
paths if there are no candidates. The script produces every path itself and
registers no fallback with bash, so a reply it cannot use offers nothing.

Keeping the directive last is what lets a script tell a whole reply from a
partial one. A program that crashed halfway through printing, or a different
program that happened to write to stdout, does not end with a directive line,
and a script that does not find one offers nothing. In the same spirit, a
`CLI11_COMPLETE_PROTO` the program does not recognise is answered with `Error`
and no candidates, which is what makes a script safe to keep across an upgrade.

Only this classification happens: nothing is parsed, no callback runs, and no
configuration file is read. `App::get_completions` exposes the same answer to
C++, taking the words of the line and the index of the one the cursor is in.

## Limitations

Where completion does not understand a line it offers nothing at all. That is
the intended failure: a wrong candidate is worse than no candidate.

The cases that reach it:

- Options addressed through the `--sub.opt` dotted notation. It needs no setter,
  so every program accepts it.
- Lines using `allow_non_standard_option_names`, `positionals_at_end`,
  `prefix_command`, `validate_positionals`, `validate_optional_arguments`, or
  `require_subcommand_max`. Each of these has to be turned on deliberately, so
  no program is affected without asking for it.
- The extra words that `allow_extras` makes legal.

Two gaps are worth knowing about on their own, because they do something worse
than offer nothing.

A dotted option is not recognised as one that takes a value, so while the dotted
word itself offers nothing, the word after it is read as a fresh word of the
outer command: asking for the value of `--start.level` offers that command's
subcommand names and positional values rather than the values `--level` accepts.

A declared value containing a space or a shell metacharacter is inserted without
quoting, which leaves a broken command line. Values that are plain words, which
is almost all of them, are unaffected.

See the
[completion.cpp](https://github.com/CLIUtils/CLI11/blob/main/examples/completion.cpp)
example, which is built to be completed against.

## Prior art

The design here builds on the good work of [cobra][] and [clap][], which have
been refining dynamic completion for years.

The reply format is cobra's. Its `ShellCompDirective` is the same bit mask under
the same names, for the same reason: a reply usually has more than one thing to
say about what the shell may do next, and a program that knows the candidates
are already in a meaningful order is also the one that knows they are not file
names.

Starting a request from an environment variable rather than a hidden subcommand
comes from clap, which is asked with `COMPLETE=bash myprog`. A subcommand would
have to be a name no program could already be using, and would show up in the
parse; a variable is invisible to everything except the run that is meant to see
it.

The differences are three, all about a script outliving the binary it was
generated for. The words of the line travel in the environment too, as they do
in Python's click, so that a binary that does not answer cannot run them.
`CLI11_COMPLETE_PROTO` lets an old script be refused rather than misread, which
matters more here than in either of those projects because a distribution may
package the script as a static file. And the `:prefix=` line moves the splitting
of `--config=/et` out of the script and into the program, which is the only side
that knows how much of the word is an option name.

Neither project's code is used here: the generated scripts and the
classification are written for CLI11, and the shared vocabulary of `COMPREPLY`
and `compopt` is bash's own.

[cobra]: https://github.com/spf13/cobra
[clap]: https://github.com/clap-rs/clap
