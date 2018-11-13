## Version 1.6.2: Help-all

This version fixes some formatting bugs with help-all. It also adds fixes for several warnings, including an experimental optional error on Clang 7. Several smaller fixes.

* Fixed help-all formatting [#163]
    * Printing help-all on nested command now fixed (App)
    * Missing space after help-all restored (Default formatter)
    * More detail printed on help all (Default formatter)
    * Help-all subcommands get indented with inner blank lines removed (Default formatter)
    * `detail::find_and_replace` added to utilities
* Fixed CMake install as subproject with `CLI11_INSTALL` flag. [#156]
* Fixed warning about local variable hiding class member with MSVC [#157]
* Fixed compile error with default settings on Clang 7 and libc++ [#158]
* Fixed special case of `--help` on subcommands (general fix planned for 1.7) [#168]
* Removing an option with links  [#179]

[#156]: https://github.com/CLIUtils/CLI11/issues/156
[#157]: https://github.com/CLIUtils/CLI11/issues/157
[#158]: https://github.com/CLIUtils/CLI11/issues/158
[#163]: https://github.com/CLIUtils/CLI11/pull/163
[#168]: https://github.com/CLIUtils/CLI11/issues/168
[#179]: https://github.com/CLIUtils/CLI11/pull/179


## Version 1.6.1: Platform fixes

This version provides a few fixes for special cases, such as mixing with `Windows.h` and better defaults
for systems like Hunter. The one new feature is the ability to produce "branded" single file output for
providing custom namespaces or custom macro names.

* Added fix and test for including Windows.h [#145]
* No longer build single file by default if main project, supports systems stuck on Python 2.6 [#149], [#151]
* Branding support for single file output [#150]

[#145]: https://github.com/CLIUtils/CLI11/pull/145
[#149]: https://github.com/CLIUtils/CLI11/pull/149
[#150]: https://github.com/CLIUtils/CLI11/pull/150
[#151]: https://github.com/CLIUtils/CLI11/pull/151

## Version 1.6: Formatting help

Added a new formatting system [#109]. You can now set the formatter on Apps. This has also simplified the internals of Apps and Options a bit by separating most formatting code.

* Added `CLI::Formatter` and `formatter` slot for apps, inherited.
* `FormatterBase` is the minimum required.
* `FormatterLambda` provides for the easy addition of an arbitrary function.
* Added `help_all` support (not added by default).

Changes to the help system (most normal users will not notice this):

* Renamed `single_name` to `get_name(false, false)` (the default).
* The old `get_name()` is now `get_name(false, true)`.
* The old `get_pname()` is now `get_name(true, false)`.
* Removed `help_*` functions.
* Protected function `_has_help_positional` removed.
* `format_help` can now be chained.
* Added getters for the missing parts of options (help no longer uses any private parts).
* Help flags now use new `short_circuit` property to simplify parsing. [#121]


New for Config file reading and writing [#121]:

* Overridable, bidirectional Config.
* ConfigINI provided and used by default.
* Renamed ini to config in many places.
* Has `config_formatter()` and `get_config_formatter()`.
* Dropped prefix argument from `config_to_str`.
* Added `ConfigItem`.
* Added an example of a custom config format using [nlohmann/json]. [#138]


Validators are now much more powerful [#118], all built in validators upgraded to the new form:

* A subclass of `CLI::Validator` is now also accepted.
* They now can set the type name to things like `PATH` and `INT in [1-4]`.
* Validators can be combined with `&` and `|`.
* Old form simple validators are still accepted.

Other changes:

* Fixing `parse(args)`'s `args` setting and ordering after parse. [#141]
* Replaced `set_custom_option` with `type_name` and `type_size` instead of `set_custom_option`. Methods return `this`. [#136]
* Dropped `set_` on Option's `type_name`, `default_str`, and `default_val`. [#136]
* Removed `set_` from App's `failure_message`, `footer`, `callback`, and `name`. [#136]
* Fixed support `N<-1` for `type_size`. [#140]
* Added `->each()` to make adding custom callbacks easier. [#126]
* Allow empty options `add_option("-n",{})` to be edited later with `each` [#142]
* Added filter argument to `get_subcommands`, `get_options`; use empty filter `{}` to avoid filtering.
* Added `get_groups()` to get groups.
* Better support for manual options with `get_option`, `set_results`, and `empty`. [#119]
* `lname` and `sname` have getters, added `const get_parent`. [#120]
* Using `add_set` will now capture L-values for sets, allowing further modification. [#113]
* Dropped duplicate way to run `get_type_name` (`get_typeval`).
* Removed `requires` in favor of `needs` (deprecated in last version). [#112]
* Const added to argv. [#126]

Backend and testing changes:

* Internally, `type_name` is now a lambda function; for sets, this reads the set live. [#116] 
* Cleaner tests without `app.reset()` (and `reset` is now `clear`). [#141]
* Better CMake policy handling. [#110]
* Includes are properly sorted. [#120]
* Testing (only) now uses submodules. [#111]

[#109]: https://github.com/CLIUtils/CLI11/pull/109
[#110]: https://github.com/CLIUtils/CLI11/pull/110
[#111]: https://github.com/CLIUtils/CLI11/pull/111
[#112]: https://github.com/CLIUtils/CLI11/pull/112
[#113]: https://github.com/CLIUtils/CLI11/issues/113
[#116]: https://github.com/CLIUtils/CLI11/pull/116
[#118]: https://github.com/CLIUtils/CLI11/pull/118
[#119]: https://github.com/CLIUtils/CLI11/pull/119
[#120]: https://github.com/CLIUtils/CLI11/pull/120
[#121]: https://github.com/CLIUtils/CLI11/pull/121
[#126]: https://github.com/CLIUtils/CLI11/pull/126
[#127]: https://github.com/CLIUtils/CLI11/pull/127
[#138]: https://github.com/CLIUtils/CLI11/pull/138
[#140]: https://github.com/CLIUtils/CLI11/pull/140
[#141]: https://github.com/CLIUtils/CLI11/pull/141
[#142]: https://github.com/CLIUtils/CLI11/pull/142

[nlohmann/json]: https://github.com/nlohmann/json

### Version 1.5.4: Optionals
This version fixes the optional search in the single file version; some macros were not yet defined when it did the search. You can define the `CLI11_*_OPTIONAL` macros to 0 if needed to eliminate the search.

### Version 1.5.3: Compiler compatibility
This version fixes older AppleClang compilers by removing the optimization for casting. The minimum version of Boost Optional supported has been clarified to be 1.58. CUDA 7.0 NVCC is now supported.

### Version 1.5.2: LICENSE in single header mode

This is a quick patch release that makes LICENSE part of the single header file, making it easier to include. Minor cleanup from codacy. No significant code changes from 1.5.1.

### Version 1.5.1: Access

This patch release adds better access to the App progromatically, to assist with writing custom converters to other formats. It also improves the help output, and uses a new feature in CLI11 1.5 to fix an old "quirk" in the way unlimited options and positionals interact.

* Make mixing unlimited positionals and options more intuitive [#102]
* Add missing getters `get_options` and `get_description` to App [#105]
* The app name now can be set, and will override the auto name if present [#105]
* Add `(REQUIRED)` for required options [#104]
* Print simple name for Needs/Excludes [#104]
* Use Needs instead of Requires in help print [#104]
* Groups now are listed in the original definition order [#106]

[#102]: https://github.com/CLIUtils/CLI11/issues/102
[#104]: https://github.com/CLIUtils/CLI11/pull/104
[#105]: https://github.com/CLIUtils/CLI11/pull/105
[#106]: https://github.com/CLIUtils/CLI11/pull/106


## Version 1.5: Optionals

This version introduced support for optionals, along with clarification and examples of custom conversion overloads. Enums now have been dropped from the automatic conversion system, allowing explicit protection for out-of-range ints (or a completely custom conversion). This version has some internal cleanup and improved support for the newest compilers. Several bugs were fixed, as well.

Note: This is the final release with `requires`, please switch to `needs`.

* Fix unlimited short options eating two values before checking for positionals when no space present [#90]
* Symmetric exclude text when excluding options, exclude can be called multiple times [#64]
* Support for `std::optional`, `std::experimental::optional`, and `boost::optional` added if `__has_include` is supported [#95]
* All macros/CMake variables now start with `CLI11_` instead of just `CLI_` [#95]
* The internal stream was not being cleared before use in some cases. Fixed. [#95]
* Using an emum now requires explicit conversion overload [#97]
* The separator `--` now is removed when it ends unlimited arguments [#100]

Other, non-user facing changes:

* Added `Macros.hpp` with better C++ mode discovery [#95]
* Deprecated macros added for all platforms
* C++17 is now tested on supported platforms [#95]
* Informational printout now added to CTest [#95]
* Better single file generation [#95]
* Added support for GTest on MSVC 2017 (but not in C++17 mode, will need next version of GTest)
* Types now have a specific size, separate from the expected number - cleaner and more powerful internally [#92]
* Examples now run as part of testing [#99]

[#64]: https://github.com/CLIUtils/CLI11/issues/64
[#90]: https://github.com/CLIUtils/CLI11/issues/90
[#92]: https://github.com/CLIUtils/CLI11/issues/92
[#95]: https://github.com/CLIUtils/CLI11/pull/95
[#97]: https://github.com/CLIUtils/CLI11/pull/97
[#99]: https://github.com/CLIUtils/CLI11/pull/99
[#100]: https://github.com/CLIUtils/CLI11/pull/100


## Version 1.4: More feedback

This version adds lots of smaller fixes and additions after the refactor in version 1.3. More ways to download and use CLI11 in CMake have been added. INI files have improved support.

* Lexical cast is now more strict than before [#68] and fails on overflow [#84]
* Added `get_parent()` to access the parent from a subcommand
* Added `ExistingPath` validator  [#73]
* `app.allow_ini_extras()` added to allow extras in INI files [#70]
* Multiline INI comments now supported
* Descriptions can now be written with `config_to_str` [#66] 
* Double printing of error message fixed [#77]
* Renamed `requires` to `needs` to avoid C++20 keyword [#75], [#82]
* MakeSingleHeader now works if outside of git [#78]
* Adding install support for CMake [#79], improved support for `find_package` [#83], [#84]
* Added support for Conan.io [#83]

[#70]: https://github.com/CLIUtils/CLI11/issues/70
[#75]: https://github.com/CLIUtils/CLI11/issues/75

[#84]: https://github.com/CLIUtils/CLI11/pull/84
[#83]: https://github.com/CLIUtils/CLI11/pull/83
[#82]: https://github.com/CLIUtils/CLI11/pull/82
[#79]: https://github.com/CLIUtils/CLI11/pull/79
[#78]: https://github.com/CLIUtils/CLI11/pull/78
[#77]: https://github.com/CLIUtils/CLI11/pull/77
[#73]: https://github.com/CLIUtils/CLI11/pull/73
[#68]: https://github.com/CLIUtils/CLI11/pull/68
[#66]: https://github.com/CLIUtils/CLI11/pull/66

## Version 1.3: Refactor

This version focused on refactoring several key systems to ensure correct behavior in the interaction of different settings. Most caveats about
features only working on the main App have been addressed, and extra arguments have been reworked. Inheritance
of defaults makes configuring CLI11 much easier without having to subclass. Policies add new ways to handle multiple arguments to match your
favorite CLI programs. Error messages and help messages are better and more flexible. Several bugs and odd behaviors in the parser have been fixed.

* Added a version macro, `CLI11_VERSION`, along with `*_MAJOR`, `*_MINOR`, and `*_PATCH`, for programmatic access to the version.
* Reworked the way defaults are set and inherited; explicit control given to user with `->option_defaults()` [#48](https://github.com/CLIUtils/CLI11/pull/48)
* Hidden options now are based on an empty group name, instead of special "hidden" keyword [#48](https://github.com/CLIUtils/CLI11/pull/48)
* `parse` no longer returns (so `CLI11_PARSE` is always usable) [#37](https://github.com/CLIUtils/CLI11/pull/37)
* Added `remaining()` and `remaining_size()` [#37](https://github.com/CLIUtils/CLI11/pull/37)
* `allow_extras` and `prefix_command` are now valid on subcommands [#37](https://github.com/CLIUtils/CLI11/pull/37)
* Added `take_last` to only take last value passed [#40](https://github.com/CLIUtils/CLI11/pull/40)
* Added `multi_option_policy` and shortcuts to provide more control than just a take last policy [#59](https://github.com/CLIUtils/CLI11/pull/59)
* More detailed error messages in a few cases [#41](https://github.com/CLIUtils/CLI11/pull/41)
* Footers can be added to help [#42](https://github.com/CLIUtils/CLI11/pull/42)
* Help flags are easier to customize [#43](https://github.com/CLIUtils/CLI11/pull/43)
* Subcommand now support groups [#46](https://github.com/CLIUtils/CLI11/pull/46)
* `CLI::RuntimeError` added, for easy exit with error codes [#45](https://github.com/CLIUtils/CLI11/pull/45) 
* The clang-format script is now no longer "hidden" [#48](https://github.com/CLIUtils/CLI11/pull/48)
* The order is now preserved for subcommands (list and callbacks) [#49](https://github.com/CLIUtils/CLI11/pull/49)
* Tests now run individually, utilizing CMake 3.10 additions if possible [#50](https://github.com/CLIUtils/CLI11/pull/50)
* Failure messages are now customizable, with a shorter default [#52](https://github.com/CLIUtils/CLI11/pull/52)
* Some improvements to error codes [#53](https://github.com/CLIUtils/CLI11/pull/53)
* `require_subcommand` now offers a two-argument form and negative values on the one-argument form are more useful [#51](https://github.com/CLIUtils/CLI11/pull/51)
* Subcommands no longer match after the max required number is obtained [#51](https://github.com/CLIUtils/CLI11/pull/51)
* Unlimited options no longer prioritize over remaining/unlimited positionals [#51](https://github.com/CLIUtils/CLI11/pull/51)
* Added `->transform` which modifies the string parsed [#54](https://github.com/CLIUtils/CLI11/pull/54)
* Changed of API in validators to `void(std::string &)` (const for users), throwing providing nicer errors [#54](https://github.com/CLIUtils/CLI11/pull/54)
* Added `CLI::ArgumentMismatch` [#56](https://github.com/CLIUtils/CLI11/pull/56) and fixed missing failure if one arg expected [#55](https://github.com/CLIUtils/CLI11/issues/55)
* Support for minimum unlimited expected arguments [#56](https://github.com/CLIUtils/CLI11/pull/56)
* Single internal arg parse function [#56](https://github.com/CLIUtils/CLI11/pull/56)
* Allow options to be disabled from INI file, rename `add_config` to `set_config` [#60](https://github.com/CLIUtils/CLI11/pull/60)

> ### Converting from CLI11 1.2:
> 
> * `app.parse` no longer returns a vector. Instead, use `app.remaining(true)`.
> * `"hidden"` is no longer a special group name, instead use `""`
> * Validators API has changed to return an error string; use `.empty()` to get the old bool back
> * Use `.set_help_flag` instead of accessing the help pointer directly (discouraged, but not removed yet)
> * `add_config` has been renamed to `set_config`
> * Errors thrown in some cases are slightly more specific

## Version 1.2: Stability

This release focuses on making CLI11 behave properly in corner cases, and with config files on the command line. This includes fixes for a variety of reported issues. A few features were added to make life easier, as well; such as a new flag callback and a macro for the parse command.

* Added functional form of flag [#33](https://github.com/CLIUtils/CLI11/pull/33), automatic on C++14
* Fixed Config file search if passed on command line [#30](https://github.com/CLIUtils/CLI11/issues/30)
* Added `CLI11_PARSE(app, argc, argv)` macro for simple parse commands (does not support returning arg)
* The name string can now contain spaces around commas [#29](https://github.com/CLIUtils/CLI11/pull/29)
* `set_default_str` now only sets string, and `set_default_val` will evaluate the default string given [#26](https://github.com/CLIUtils/CLI11/issues/26)
* Required positionals now take priority over subcommands [#23](https://github.com/CLIUtils/CLI11/issues/23)
* Extra requirements enforced by Travis

## Version 1.1: Feedback

This release incorporates feedback from the release announcement. The examples are slowly being expanded, some corner cases improved, and some new functionality for tricky parsing situations.

* Added simple support for enumerations, allow non-printable objects [#12](https://github.com/CLIUtils/CLI11/issues/12)
* Added `app.parse_order()` with original parse order ([#13](https://github.com/CLIUtils/CLI11/issues/13), [#16](https://github.com/CLIUtils/CLI11/pull/16)) 
* Added `prefix_command()`, which is like `allow_extras` but instantly stops and returns. ([#8](https://github.com/CLIUtils/CLI11/issues/8), [#17](https://github.com/CLIUtils/CLI11/pull/17)) 
* Removed Windows warning ([#10](https://github.com/CLIUtils/CLI11/issues/10), [#20](https://github.com/CLIUtils/CLI11/pull/20))
* Some improvements to CMake, detect Python and no dependencies on Python 2 (like Python 3) ([#18](https://github.com/CLIUtils/CLI11/issues/18), [#21](https://github.com/CLIUtils/CLI11/pull/21))

## Version 1.0: Official release

This is the first stable release for CLI11. Future releases will try to remain backward compatible and will follow semantic versioning if possible. There were a few small changes since version 0.9:

* Cleanup using `clang-tidy` and `clang-format`
* Small improvements to Timers, easier to subclass Error
* Move to 3-Clause BSD license

## Version 0.9: Polish

This release focused on cleaning up the most exotic compiler warnings, fixing a few oddities of the config parser, and added a more natural method to check subcommands.

* Better CMake named target (CLI11)
* More warnings added, fixed
* Ini output now includes `=false` when `default_also` is true
* Ini no longer lists the help pointer
* Added test for inclusion in multiple files and linking, fixed issues (rarely needed for CLI, but nice for tools)
* Support for complex numbers
* Subcommands now test true/false directly or with `->parsed()`, cleaner parse

## Version 0.8: CLIUtils

This release moved the repository to the CLIUtils master organization.

* Moved to CLIUtils on GitHub
* Fixed docs build and a few links

## Version 0.7: Code coverage 100%

Lots of small bugs fixed when adding code coverage, better in edge cases. Much more powerful ini support.

* Allow comments in ini files (lines starting with `;`)
* Ini files support flags, vectors, subcommands
* Added CodeCov code coverage reports
* Lots of small bugfixes related to adding tests to increase coverage to 100%
* Error handling now uses scoped enum in errors
* Reparsing rules changed a little to accommodate Ini files. Callbacks are now called when parsing INI, and reset any time results are added.
* Adding extra utilities in full version only, `Timer` (not needed for parsing, but useful for general CLI applications).
* Better support for custom `add_options` like functions.

## Version 0.6: Cleanup

Lots of cleanup and docs additions made it into this release. Parsing is simpler and more robust; fall through option added and works as expected; much more consistent variable names internally.

* Simplified parsing to use `vector<string>` only
* Fixed fallthrough, made it optional as well (default: off): `.fallthrough()`.
* Added string versions of `->requires()` and `->excludes()` for consistency.
* Renamed protected members for internal consistency, grouped docs.
* Added the ability to add a number to `.require_subcommand()`.

## Version 0.5: Windows support

* Allow `Hidden` options.
* Throw `OptionAlreadyAdded` errors for matching subcommands or options, with ignore-case included, tests
* `->ignore_case()` added to subcommands, options, and `add_set_ignore_case`. Subcommands inherit setting from parent App on creation.
* Subcommands now can be "chained", that is, left over arguments can now include subcommands that then get parsed. Subcommands are now a list (`get_subcommands`). Added `got_subcommand(App_or_name)` to check for subcommands.
* Added `.allow_extras()` to disable error on failure. Parse returns a vector of leftover options. Renamed error to `ExtrasError`, and now triggers on extra options too.
* Added `require_subcommand` to `App`, to simplify forcing subcommands. Do **not** do `add_subcommand()->require_subcommand`, since that is the subcommand, not the master `App`.
* Added printout of ini file text given parsed options, skips flags.
* Support for quotes and spaces in ini files
* Fixes to allow support for Windows (added Appveyor) (Uses `-`, not `/` syntax)

## Version 0.4: Ini support

* Updates to help print
* Removed `run`, please use `parse` unless you subclass and add it
* Supports ini files mixed with command line, tested
* Added Range for further Plumbum compatibility
* Added function to print out ini file

## Version 0.3: Plumbum compatibility

* Added `->requires`, `->excludes`, and `->envname` from [Plumbum](http://plumbum.readthedocs.io/en/latest/)
* Supports `->mandatory` from Plubmum
* More tests for help strings, improvements in formatting
* Support type and set syntax in positionals help strings
* Added help groups, with `->group("name")` syntax
* Added initial support for ini file reading with `add_config` option.
* Supports GCC 4.7 again
* Clang 3.5 now required for tests due to googlemock usage, 3.4 should still work otherwise
* Changes `setup` for an explicit help bool in constructor/`add_subcommand`

## Version 0.2: Leaner and meaner

* Moved to simpler syntax, where `Option` pointers are returned and operated on
* Removed `make_` style options
* Simplified Validators, now only requires `->check(function)`
* Removed Combiners
* Fixed pointers to Options, stored in `unique_ptr` now
* Added `Option_p` and `App_p`, mostly for internal use
* Startup sequence, including help flag, can be modified by subclasses

## Version 0.1: First release

First release before major cleanup. Still has make syntax and combiners; very clever syntax but not the best or most commonly expected way to work.

