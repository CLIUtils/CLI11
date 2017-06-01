## Version 1.0
* Cleanup using `clang-tidy` and `clang-format`
* Small improvements to Timers, easier to subclass Error
* Move to 3-Clause BSD license

## Version 0.9

* Better CMake named target (CLI11)
* More warnings added, fixed
* Ini output now includes `=false` when `default_also` is true
* Ini no longer lists the help pointer
* Added test for inclusion in multiple files and linking, fixed issues (rarely needed for CLI, but nice for tools)
* Support for complex numbers
* Subcommands now test true/false directly or with `->parsed()`, cleaner parse

## Version 0.8

* Moved to CLIUtils on GitHub
* Fixed docs build and a few links

## Version 0.7

* Allow comments in ini files (lines starting with `;`)
* Ini files support flags, vectors, subcommands
* Added CodeCov code coverage reports
* Lots of small bugfixes related to adding tests to increase coverage to 100%
* Error handling now uses scoped enum in errors
* Reparsing rules changed a little to accommodate Ini files. Callbacks are now called when parsing INI, and reset any time results are added.
* Adding extra utilities in full version only, `Timer` (not needed for parsing, but useful for general CLI applications).
* Better support for custom `add_options` like functions.

## Version 0.6

* Simplified parsing to use `vector<string>` only
* Fixed fallthrough, made it optional as well (default: off): `.fallthrough()`.
* Added string versions of `->requires()` and `->excludes()` for consistency.
* Renamed protected members for internal consistency, grouped docs.
* Added the ability to add a number to `.require_subcommand()`.

## Version 0.5

* Allow `Hidden` options.
* Throw `OptionAlreadyAdded` errors for matching subcommands or options, with ignore-case included, tests
* `->ignore_case()` added to subcommands, options, and `add_set_ignore_case`. Subcommands inherit setting from parent App on creation.
* Subcommands now can be "chained", that is, left over arguments can now include subcommands that then get parsed. Subcommands are now a list (`get_subcommands`). Added `got_subcommand(App_or_name)` to check for subcommands.
* Added `.allow_extras()` to disable error on failure. Parse returns a vector of leftover options. Renamed error to `ExtrasError`, and now triggers on extra options too.
* Added `require_subcommand` to `App`, to simplify forcing subcommands. Do **not** do `add_subcommand()->require_subcommand`, since that is the subcommand, not the master `App`.
* Added printout of ini file text given parsed options, skips flags.
* Support for quotes and spaces in ini files
* Fixes to allow support for Windows (added Appveyor) (Uses `-`, not `/` syntax)

## Version 0.4

* Updates to help print
* Removed `run`, please use `parse` unless you subclass and add it
* Supports ini files mixed with command line, tested
* Added Range for further Plumbum compatibility
* Added function to print out ini file

## Version 0.3

* Added `->requires`, `->excludes`, and `->envname` from [Plumbum](http://plumbum.readthedocs.io/en/latest/)
* Supports `->mandatory` from Plubmum
* More tests for help strings, improvements in formatting
* Support type and set syntax in positionals help strings
* Added help groups, with `->group("name")` syntax
* Added initial support for ini file reading with `add_config` option.
* Supports GCC 4.7 again
* Clang 3.5 now required for tests due to googlemock usage, 3.4 should still work otherwise
* Changes `setup` for an explicit help bool in constructor/`add_subcommand`

## Version 0.2

* Moved to simpler syntax, where `Option` pointers are returned and operated on
* Removed `make_` style options
* Simplified Validators, now only requires `->check(function)`
* Removed Combiners
* Fixed pointers to Options, stored in `unique_ptr` now
* Added `Option_p` and `App_p`, mostly for internal use
* Startup sequence, including help flag, can be modified by subclasses

## Version 0.1

Initial version


