# Example programs {#example-programs}

The [`examples/`][examples] directory holds small, complete programs. Each one
builds and runs on its own, so you can copy a file into your project and change
it. Build them with the `CLI11_BUILD_EXAMPLES` CMake option, which is on by
default when CLI11 is the top level project.

Read @ref book-an-advanced-example for a step by step walkthrough of a larger
program.

## Start here

`minimal.cpp` is the smallest program that parses and exits correctly. Use it as
a starting point for a new application:

\include minimal.cpp

`simple.cpp` adds options, a flag, and a version flag, and shows how to read the
values and the counts back after the parse:

\include simple.cpp

## Options and flags

| Example                      | Shows                                                        |
| ---------------------------- | ------------------------------------------------------------ |
| [array_option.cpp][]         | An option that fills a `std::array`                          |
| [digit_args.cpp][]           | Numbered flags (`-1` to `-9`) with values attached           |
| [enum.cpp][]                 | An option that takes an `enum class` value                   |
| [enum_ostream.cpp][]         | A custom `operator<<` for an `enum class` in help and errors |
| [ranges.cpp][]               | Options that accept a range of values                        |
| [retired.cpp][]              | Retired and deprecated options                               |
| [inter_argument_order.cpp][] | Recovery of the original order of unlimited arguments        |
| [custom_parse.cpp][]         | A custom lexical cast for a user type                        |

## Validators

| Example                       | Shows                                              |
| ----------------------------- | -------------------------------------------------- |
| [validators.cpp][]            | The built in `ExistingFile` and `Range` validators |
| [custom_validator.cpp][]      | A `CLI::Validator` subclass                        |
| [date_validator.cpp][]        | A `CLI::CustomValidator` built from a lambda       |
| [positional_validation.cpp][] | Validators that select between positionals         |

## Subcommands and groups

| Example                    | Shows                                                  |
| -------------------------- | ------------------------------------------------------ |
| [subcommands.cpp][]        | Basic subcommands and `--help-all`                     |
| [subcom_help.cpp][]        | A required argument on a subcommand                    |
| [subcom_partitioned.cpp][] | Subcommands built as separate `CLI::App_p` objects     |
| [subcom_in_files][]        | A subcommand defined in its own source and header file |
| [shapes.cpp][]             | Repeated subcommands with immediate callbacks          |
| [nested.cpp][]             | Nested subcommands                                     |
| [groups.cpp][]             | Help groups                                            |
| [option_groups.cpp][]      | Option groups and the requirements between them        |
| [positional_arity.cpp][]   | Option groups that select on the number of positionals |

## Help output

| Example             | Shows                                            |
| ------------------- | ------------------------------------------------ |
| [formatter.cpp][]   | A `CLI::Formatter` subclass                      |
| [help_usage.cpp][]  | A custom usage line and wide character support   |
| [modhelp.cpp][]     | Help printed after the parse, with option values |
| [close_match.cpp][] | A "did you mean" message for an unknown option   |

## Configuration files

| Example            | Shows                                                           |
| ------------------ | --------------------------------------------------------------- |
| [config_app.cpp][] | Reading a configuration file and printing the result            |
| [json.cpp][]       | A JSON configuration file, with [nlohmann/json][] as the parser |

## Passing arguments on

| Example                      | Shows                                           |
| ---------------------------- | ----------------------------------------------- |
| [prefix_command.cpp][]       | `prefix_command` on the main application        |
| [arg_capture.cpp][]          | `prefix_command` on a subcommand, with an alias |
| [callback_passthrough.cpp][] | A callback that parses the remaining arguments  |
| [testEXE.cpp][]              | Arguments collected for another program         |

[examples]: https://github.com/CLIUtils/CLI11/tree/main/examples
[nlohmann/json]: https://github.com/nlohmann/json
[arg_capture.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/arg_capture.cpp
[array_option.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/array_option.cpp
[callback_passthrough.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/callback_passthrough.cpp
[close_match.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/close_match.cpp
[config_app.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/config_app.cpp
[custom_parse.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/custom_parse.cpp
[custom_validator.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/custom_validator.cpp
[date_validator.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/date_validator.cpp
[digit_args.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/digit_args.cpp
[enum.cpp]: https://github.com/CLIUtils/CLI11/blob/main/examples/enum.cpp
[enum_ostream.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/enum_ostream.cpp
[formatter.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/formatter.cpp
[groups.cpp]: https://github.com/CLIUtils/CLI11/blob/main/examples/groups.cpp
[help_usage.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/help_usage.cpp
[inter_argument_order.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/inter_argument_order.cpp
[json.cpp]: https://github.com/CLIUtils/CLI11/blob/main/examples/json.cpp
[modhelp.cpp]: https://github.com/CLIUtils/CLI11/blob/main/examples/modhelp.cpp
[nested.cpp]: https://github.com/CLIUtils/CLI11/blob/main/examples/nested.cpp
[option_groups.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/option_groups.cpp
[positional_arity.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/positional_arity.cpp
[positional_validation.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/positional_validation.cpp
[prefix_command.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/prefix_command.cpp
[ranges.cpp]: https://github.com/CLIUtils/CLI11/blob/main/examples/ranges.cpp
[retired.cpp]: https://github.com/CLIUtils/CLI11/blob/main/examples/retired.cpp
[shapes.cpp]: https://github.com/CLIUtils/CLI11/blob/main/examples/shapes.cpp
[subcom_help.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/subcom_help.cpp
[subcom_in_files]:
  https://github.com/CLIUtils/CLI11/tree/main/examples/subcom_in_files
[subcom_partitioned.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/subcom_partitioned.cpp
[subcommands.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/subcommands.cpp
[testEXE.cpp]: https://github.com/CLIUtils/CLI11/blob/main/examples/testEXE.cpp
[validators.cpp]:
  https://github.com/CLIUtils/CLI11/blob/main/examples/validators.cpp
