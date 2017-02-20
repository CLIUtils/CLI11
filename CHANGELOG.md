## Version 0.5 (in progress)

* Added `->allow_extras()` to disable error on failure. Parse returns a vector of leftover options. Renamed error to `ExtrasError`, and now triggers on extra options too.
* Added `require_subcommand` to `App`, to simplify forcing subcommands. Do not "chain" with `add_subcommand`, since that is the subcommand, not the master `App`.
* Added printout of ini file text given parsed options, skips flags.
* Support for quotes and spaces in ini files
* Fixes to allow support for Windows (added Appveyor) (Use `-`, not `/` syntax)

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


