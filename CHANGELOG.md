# Changelog

## Unreleased

## Version 2.6.0

### Added

- Added option to align long options with a ratio via
  `long_option_alignment_ratio` in the formatter. This allows more control over
  help output formatting. [#1185][]
- Added support for `std::string_view` in the `as<T>` method on options.
  [#1187][]
- Added flags on the formatter to disable formatting for the description and
  footer, allowing custom formatting such as word art. [#1150][]
- Added subcommand prefix matching as a modifier to `CLI::App`. Also included an
  example of close matching logic. [#1152][]
- Added additional fuzzing mechanics, including fuzzing subcommands, and
  improved handling of edge cases. [#1170][]
- Added new tests for array options and fixed ambiguity between tuple and
  container conversions. [#1136][]
- Added ability to use rvalue references in `add_flag` descriptions. [#1173][]
- Added CMake presets for default and tidy builds. [#1181][]
- Added several validator examples and documentation [#1192][]
- Added permission validators for files and directories [#1203][]

### Changed

- Moved several of the validators to `ExtraValidators.hpp` and
  `ExtraValidators_inl.hpp` files, The compilation of these nonessential
  validators can be disabled by setting `CLI11_DISABLE_EXTRA_VALIDATORS` to
  `OFF`. Future additional validators will be behind a compile flag
  `CLI11_ENABLE_EXTRA_VALIDATORS`. All non-essential validators will be under
  this option with version 3.0. [#1192][]
- Updated processing order: requirements are now checked before callbacks,
  avoiding unexpected side effects. [#1186][]
- Updated minimum required CMake version to 3.14+. [#1182][]
- Improved Meson build: support for building shared precompiled libraries,
  pkgconfig, and header installation. [#1167][]
- Improved fuzzing tests with new failure cases and extended coverage. [#1164][]
- Updated CI to remove deprecated images and add new ones (Windows-2022/2025,
  arm64, FreeBSD). [#1172][], [#1178][]
- Updated license file to include the correct version number for packagers.
  [#1180][]

### Fixed

- Fixed issue with IPV4 validator where it would allow a trailing `.`. [#1192][]
- Fixed edge case where a missing config file and no default caused a segfault.
  [#1199][]
- Fixed issue with TOML multiline arrays when the first line contained only a
  single character. [#1196][]
- Fixed default value conversion errors when locales added thousands separators.
  [#1160][]
- Fixed multiple footer printing in help output for option groups. [#1161][]
- Fixed incorrect argument order in extras error messages. [#1162][]
- Fixed reversed argument order in unexpected argument error messages. [#1158][]
- Fixed ambiguity with `vector<array>` options. [#1147][]
- Fixed bug parsing negative floating point values without a leading zero.
  [#1140][]
- Fixed spelling mistake in `Error.hpp`. [#1129][]
- Fixed compilation issue with MSVC 2017. [#1143][]
- Fixed issue with default strings of arrays in config output. [#1155][]
- Fixed fuzzing issues with NaNs and certain error pathways. [#1138][]
- Fixed fuzzer misinterpreting `--sub1.-` as a short option. [#1148][]
- Fixed issue where parse_order was not cleared on reset. [#1218][]
- modify code to make compatible with /GR- option in MSVC [#1206][]

[#1129]: https://github.com/CLIUtils/CLI11/pull/1129
[#1136]: https://github.com/CLIUtils/CLI11/pull/1136
[#1138]: https://github.com/CLIUtils/CLI11/pull/1138
[#1140]: https://github.com/CLIUtils/CLI11/pull/1140
[#1143]: https://github.com/CLIUtils/CLI11/pull/1143
[#1147]: https://github.com/CLIUtils/CLI11/pull/1147
[#1148]: https://github.com/CLIUtils/CLI11/pull/1148
[#1150]: https://github.com/CLIUtils/CLI11/pull/1150
[#1152]: https://github.com/CLIUtils/CLI11/pull/1152
[#1155]: https://github.com/CLIUtils/CLI11/pull/1155
[#1158]: https://github.com/CLIUtils/CLI11/pull/1158
[#1160]: https://github.com/CLIUtils/CLI11/pull/1160
[#1161]: https://github.com/CLIUtils/CLI11/pull/1161
[#1162]: https://github.com/CLIUtils/CLI11/pull/1162
[#1164]: https://github.com/CLIUtils/CLI11/pull/1164
[#1167]: https://github.com/CLIUtils/CLI11/pull/1167
[#1170]: https://github.com/CLIUtils/CLI11/pull/1170
[#1172]: https://github.com/CLIUtils/CLI11/pull/1172
[#1173]: https://github.com/CLIUtils/CLI11/pull/1173
[#1178]: https://github.com/CLIUtils/CLI11/pull/1178
[#1180]: https://github.com/CLIUtils/CLI11/pull/1180
[#1181]: https://github.com/CLIUtils/CLI11/pull/1181
[#1182]: https://github.com/CLIUtils/CLI11/pull/1182
[#1185]: https://github.com/CLIUtils/CLI11/pull/1185
[#1186]: https://github.com/CLIUtils/CLI11/pull/1186
[#1187]: https://github.com/CLIUtils/CLI11/pull/1187
[#1192]: https://github.com/CLIUtils/CLI11/pull/1192
[#1196]: https://github.com/CLIUtils/CLI11/pull/1196
[#1199]: https://github.com/CLIUtils/CLI11/pull/1199
[#1203]: https://github.com/CLIUtils/CLI11/pull/1203
[#1206]: https://github.com/CLIUtils/CLI11/pull/1206
[#1218]: https://github.com/CLIUtils/CLI11/pull/1218

## Version 2.5: Help Formatter

This version adds a new formatter with improved control capabilities and output
aligned with standards for help output. It also adds a modifier to enable use of
non-standard option names, along with several bug fixes for edge cases in string
and config file parsing.

- Improved help formatter [#866][], better aligns help generation with UNIX
  standards and allows use in help2man. [#1093][]
- Added mechanism to allow option groups to be hidden and all options to be
  considered part of the parent for help display [#1039][]
- Added a modifier to allow non-standard single flag option names such as
  `-option`. [#1078][]
- Added modifier for subcommands to disable fallthrough, which can resolve some
  issues with positional arguments [#1073][]
- Added polish to config file output, removing unnecessary output and adding
  modifier to control output of default values [#1075][]
- Added ability to specify pair/tuple defaults and improved parsing [#1081][]
- Bugfix: Take the configurability of an option name into account when
  determining naming conflicts [#1049][]
- Bugfix: Fixed an issue where an extra subcommand header was being printed in
  the output [#1058][]
- Bugfix: Added additional fuzzing tests and fixes for a bug in escape string
  processing, and resolved inconsistencies in the handling of `{}` between
  command line parsing and config file parsing. [#1060][]
- Bugfix: Improved handling of ambiguities in vector input processing for config
  files, specifically in the case of vector of vector inputs. [#1069][]
- Bugfix: Fixed an issue in the handling of uint8_t enums, and issues related to
  single element tuples [#1087][]
- Bugfix: Fixed an issue with binary strings containing a `\x` [#1097][]
- Bugfix: Moved the help generation priority so it triggers before config file
  processing [#1106][]
- Bugfix: Fixed an issue where max/min on positionals was not being respected
  and optional positionals were being ignored [#1108][]
- Bugfix: Fixed an issue with strings which started and ended with brackets
  being misinterpreted as vectors. Parsing now has special handling of strings
  which start with `[[` [#1110][]
- Bugfix: Fixed some macros for support in C++26 related to wide string parsing
  [#1113][]
- Bugfix: Allowed trailing spaces on numeric string conversions [#1115][]
- Docs: Updated pymod.find_installation to find Python in meson.build [#1076][]
- Docs: Added example for transform validators [#689][]
- Docs: Fixed several spelling mistakes [#1101][]
- Backend: Updated copyright dates to 2025 [#1112][]
- Backend: Updated CMAKE minimum version to 3.10 [#1084][]

[#1039]: https://github.com/CLIUtils/CLI11/pull/1039
[#1049]: https://github.com/CLIUtils/CLI11/pull/1049
[#1058]: https://github.com/CLIUtils/CLI11/pull/1058
[#1060]: https://github.com/CLIUtils/CLI11/pull/1060
[#1069]: https://github.com/CLIUtils/CLI11/pull/1069
[#866]: https://github.com/CLIUtils/CLI11/pull/866
[#1073]: https://github.com/CLIUtils/CLI11/pull/1073
[#1075]: https://github.com/CLIUtils/CLI11/pull/1075
[#689]: https://github.com/CLIUtils/CLI11/pull/689
[#1076]: https://github.com/CLIUtils/CLI11/pull/1076
[#1078]: https://github.com/CLIUtils/CLI11/pull/1078
[#1081]: https://github.com/CLIUtils/CLI11/pull/1081
[#1084]: https://github.com/CLIUtils/CLI11/pull/1084
[#1087]: https://github.com/CLIUtils/CLI11/pull/1087
[#1093]: https://github.com/CLIUtils/CLI11/pull/1093
[#1097]: https://github.com/CLIUtils/CLI11/pull/1097
[#1101]: https://github.com/CLIUtils/CLI11/pull/1101
[#1106]: https://github.com/CLIUtils/CLI11/pull/1106
[#1108]: https://github.com/CLIUtils/CLI11/pull/1108
[#1110]: https://github.com/CLIUtils/CLI11/pull/1110
[#1112]: https://github.com/CLIUtils/CLI11/pull/1112
[#1113]: https://github.com/CLIUtils/CLI11/pull/1113
[#1115]: https://github.com/CLIUtils/CLI11/pull/1115

## Version 2.4: Unicode and TOML support

This version adds Unicode support, support for TOML standard including multiline
strings, digit separators, string escape sequences,and dot notation. An initial
round of a fuzzer was added to testing which has caught several bugs related to
config file processing, and a few other edge cases not previously observed.

NOTE: The fuzzer fixes have had some previously unrecognized changes to allowed positional names.  Positional names with spaces are no longer allowed.  This was not explicitely allowed but not disallowed either, and now it is no longer allowed as these options may be called in config file parsing.  

- Add Unicode support and bug fixes [#804][], [#923][], [#876][], [#848][],
  [#832][], [#987][]
- Match TOML standard for string and numerical entries, multiline strings
  [#968][], [#967][],[#964][], [#935][]
- Add validation for environmental variables [#926][]
- Add an escape string transform [#970][]
- Add A REVERSE multi-option policy to support multiple config files and other
  applications [#918][]
- Add usage message replacement [#768][]
- Allow using dot notation for subcommand arguments such as `--sub1.field`
  [#789][]
- Bugfix: Fuzzing tests and fixes [#930][], [#905][], [#874][], [#846][]
- Bugfix: Missing coverage tests [#928][]
- Bugfix: CMake package and package config tests and fixes [#916][]
- Bugfix: Support for Windows ARM compilation and tests [#913][], [#914][]
- Bugfix: Environmental variable checks in non-triggered subcommands [#904][]
- Bugfix: Environmental variables were not being correctly process by config
  pointer [#891][]
- Bugfix: Undefined behavior in `sum_string_vector` [#893][]
- Bugfix: Warnings and updates for CUDA 11 support [#851][]
- Backend: Add tests for newer compilers (lost with Travis CI) [#972][]
- Backend: Increase minimum CMake to 3.5 [#898][]
- Backend: Remove integrated Conan support (provided now by Conan center)
  [#853][]
- Tests: Support Catch2 Version 3 [#896][], [#980][]

[#768]: https://github.com/CLIUtils/CLI11/pull/768
[#789]: https://github.com/CLIUtils/CLI11/pull/789
[#804]: https://github.com/CLIUtils/CLI11/pull/804
[#832]: https://github.com/CLIUtils/CLI11/pull/832
[#846]: https://github.com/CLIUtils/CLI11/pull/846
[#848]: https://github.com/CLIUtils/CLI11/pull/848
[#851]: https://github.com/CLIUtils/CLI11/pull/851
[#853]: https://github.com/CLIUtils/CLI11/pull/853
[#874]: https://github.com/CLIUtils/CLI11/pull/874
[#876]: https://github.com/CLIUtils/CLI11/pull/876
[#891]: https://github.com/CLIUtils/CLI11/pull/891
[#893]: https://github.com/CLIUtils/CLI11/pull/893
[#896]: https://github.com/CLIUtils/CLI11/pull/896
[#898]: https://github.com/CLIUtils/CLI11/pull/898
[#904]: https://github.com/CLIUtils/CLI11/pull/904
[#905]: https://github.com/CLIUtils/CLI11/pull/905
[#913]: https://github.com/CLIUtils/CLI11/pull/913
[#914]: https://github.com/CLIUtils/CLI11/pull/914
[#916]: https://github.com/CLIUtils/CLI11/pull/916
[#918]: https://github.com/CLIUtils/CLI11/pull/918
[#923]: https://github.com/CLIUtils/CLI11/pull/923
[#926]: https://github.com/CLIUtils/CLI11/pull/926
[#928]: https://github.com/CLIUtils/CLI11/pull/928
[#930]: https://github.com/CLIUtils/CLI11/pull/930
[#935]: https://github.com/CLIUtils/CLI11/pull/935
[#964]: https://github.com/CLIUtils/CLI11/pull/964
[#967]: https://github.com/CLIUtils/CLI11/pull/967
[#968]: https://github.com/CLIUtils/CLI11/pull/968
[#970]: https://github.com/CLIUtils/CLI11/pull/970
[#972]: https://github.com/CLIUtils/CLI11/pull/972
[#980]: https://github.com/CLIUtils/CLI11/pull/980
[#987]: https://github.com/CLIUtils/CLI11/pull/987

### Version 2.4.1: Missing header

A transitive include that might be present in some standard libraries is now
included directly. This also fixes a test on architectures that need libatomic
linked and fix an inadvertent breaking change regarding unused defaults for
config files

- Bugfix: Include cstdint [#996][]
- Bugfix: Fix change in operation of config_ptr with unused default in the count
  method [#1003][]
- Tests: Include libatomic if required for fuzzing test [#1000][]

[#996]: https://github.com/CLIUtils/CLI11/pull/996
[#1000]: https://github.com/CLIUtils/CLI11/pull/1000
[#1003]: https://github.com/CLIUtils/CLI11/pull/1003

### Version 2.4.2: Build systems

This version improves support for alternative build systems, like Meson and
Bazel. The single-include file now is in its own subdirectory. Several smaller
fixes as well.

- Meson: fixes, cleanups, and modernizations [#1024][] & [#1025][]
- Support building with Bazel [#1033][]
- Restore non-arch dependent path for the pkgconfig file [#1012][]
- Add `get_subcommand_no_throw` [#1016][]
- Move single file to `single-include` folder [#1030][] & [#1036][]
- Fixed `app.set_failure_message(...)` -> `app.failure_message(...)` [#1018][]
- Add IWYU pragmas [#1008][]
- Fix internal header include paths [#1011][]
- Improved clarity in `RequiredError` [#1029][]
- Added ability to use lexical_cast overloads constrained with enable_if
  [#1021][]
- Bug fixes in latest release related to environmental variable parsing from
  option groups and unrecognized fields in a config file [#1005][]

[#1005]: https://github.com/CLIUtils/CLI11/pull/1005
[#1008]: https://github.com/CLIUtils/CLI11/pull/1008
[#1011]: https://github.com/CLIUtils/CLI11/pull/1011
[#1012]: https://github.com/CLIUtils/CLI11/pull/1012
[#1016]: https://github.com/CLIUtils/CLI11/pull/1016
[#1018]: https://github.com/CLIUtils/CLI11/pull/1018
[#1021]: https://github.com/CLIUtils/CLI11/pull/1021
[#1024]: https://github.com/CLIUtils/CLI11/pull/1024
[#1025]: https://github.com/CLIUtils/CLI11/pull/1025
[#1029]: https://github.com/CLIUtils/CLI11/pull/1029
[#1030]: https://github.com/CLIUtils/CLI11/pull/1030
[#1033]: https://github.com/CLIUtils/CLI11/pull/1033
[#1036]: https://github.com/CLIUtils/CLI11/pull/1036

## Version 2.3: Precompilation Support

This version adds a pre-compiled mode to CLI11, which allows you to precompile
the library, saving time on incremental rebuilds, making CLI11 more competitive
on compile time with classic compiled CLI libraries. The header-only mode is
still default, and is not yet distributed via binaries.

- Add `CLI11_PRECOMPILED` as an option [#762][]
- Bugfix: Include `<functional>` in `FormatterFwd` [#727][]
- Bugfix: Add missing `Macros.hpp` to `Error.hpp` [#755][]
- Bugfix: Fix subcommand callback trigger [#733][]
- Bugfix: Variable rename to avoid warning [#734][]
- Bugfix: `split_program_name` single file name error [#740][]
- Bugfix: Better support for min/max overrides on MSVC [#741][]
- Bugfix: Support MSVC 2022 [#748][]
- Bugfix: Support negated flag in config file [#775][]
- Bugfix: Better errors for some confusing config file situations [#781][]
- Backend: Restore coverage testing (lost with Travis CI) [#747][]

[#727]: https://github.com/CLIUtils/CLI11/pull/727
[#733]: https://github.com/CLIUtils/CLI11/pull/733
[#734]: https://github.com/CLIUtils/CLI11/pull/734
[#740]: https://github.com/CLIUtils/CLI11/pull/740
[#741]: https://github.com/CLIUtils/CLI11/pull/741
[#747]: https://github.com/CLIUtils/CLI11/pull/747
[#748]: https://github.com/CLIUtils/CLI11/pull/748
[#755]: https://github.com/CLIUtils/CLI11/pull/755
[#762]: https://github.com/CLIUtils/CLI11/pull/762
[#775]: https://github.com/CLIUtils/CLI11/pull/775
[#781]: https://github.com/CLIUtils/CLI11/pull/781

### Version 2.3.1: Missing implementation

A function implementation was missing after the pre-compile move, missed due to
the fact we lost 100% after losing coverage checking. We are working on filling
out 100% coverage again to ensure this doesn't happen again!

- Bugfix: `App::get_option_group` implementation missing [#793][]
- Bugfix: Fix spacing when setting an empty footer [#796][]
- Bugfix: Address Klocwork static analysis checking issues [#785][]

[#785]: https://github.com/CLIUtils/CLI11/pull/785
[#793]: https://github.com/CLIUtils/CLI11/pull/793
[#796]: https://github.com/CLIUtils/CLI11/pull/796

### Version 2.3.2: Minor maintenance

This version provides a few fixes collected over the last three months before
adding features for 2.4.

- Bugfix: Consistently use ADL for `lexical_cast`, making it easier to extend
  for custom template types [#820][]
- Bugfix: Tweak the parsing of files for flags with `disable_flag_override`
  [#800][]
- Bugfix: Handle out of bounds long long [#807][]
- Bugfix: Spacing of `make_description` min option output [#808][]
- Bugfix: Print last parsed subcommand's help message [#822][]
- Bugfix: Avoid floating point warning in GCC 12 [#803][]
- Bugfix: Fix a few gcc warnings [#813][]
- Backend: Max CMake tested 3.22 -> 3.24 [#823][]

[#800]: https://github.com/CLIUtils/CLI11/pull/800
[#803]: https://github.com/CLIUtils/CLI11/pull/803
[#807]: https://github.com/CLIUtils/CLI11/pull/807
[#808]: https://github.com/CLIUtils/CLI11/pull/808
[#813]: https://github.com/CLIUtils/CLI11/pull/813
[#820]: https://github.com/CLIUtils/CLI11/pull/820
[#822]: https://github.com/CLIUtils/CLI11/pull/822
[#823]: https://github.com/CLIUtils/CLI11/pull/823

## Version 2.2: Option and Configuration Flexibility

New features include support for output of an empty vector, a summing option
policy that can be applied more broadly, and an option to validate optional
arguments to discriminate from positional arguments. A new validator to check
for files on a default path is included to allow one or more default paths for
configuration files or other file arguments. A number of bug fixes and code
cleanup for various build configurations. Clean up of some error outputs and
extension of existing capability to new types or situations.

There is a possible minor breaking change in behavior of certain types which
wrapped an integer, such as `std::atomic<int>` or `std::optional<int>` when used
in a flag. The default behavior is now as a single argument value vs. summing
all the arguments. The default summing behavior is now restricted to pure
integral types, int64_t, int, uint32_t, etc. Use the new `sum` multi option
policy to revert to the older behavior. The summing behavior on wrapper types
was not originally intended.

- Add `MultiOptionPolicy::Sum` and refactor the `add_flag` to fix a bug when
  using `std::optional<bool>` as type. [#709][]
- Add support for an empty vector result in TOML and as a default string.
  [#660][]
- Add `.validate_optional_arguments()` to support discriminating positional
  arguments from vector option arguments. [#668][]
- Add `CLI::FileOnDefaultPath` to check for files on a specified default path.
  [#698][]
- Change default value display in help messages from `=XXXX` to `[XXXXX]` to
  make it clearer. [#666][]
- Modify the Range Validator to support additional types and clean up the error
  output. [#690][]
- Bugfix: The trigger on parse modifier did not work on positional argument.s
  [#713][]
- Bugfix: The single header file generation was missing custom namespace
  generation. [#707][]
- Bugfix: Clean up File Error handling in the argument processing. [#678][]
- Bugfix: Fix a stack overflow error if nameless commands had fallthrough.
  [#665][]
- Bugfix: A subcommand callback could be executed multiple times if it was a
  member of an option group. [#666][]
- Bugfix: Fix an issue with vectors of multi argument types where partial
  argument sets did not result in an error. [#661][]
- Bugfix: Fix an issue with type the template matching on C++20 and add some CI
  builds for C++20. [#663][]
- Bugfix: Fix typo in C++20 detection on MSVC. [#706][]
- Bugfix: An issue where the detection of RTTI being disabled on certain MSVC
  platforms did not disable the use of dynamic cast calls. [#666][]
- Bugfix: Resolve strict-overflow warning on some GCC compilers. [#666][]
- Backend: Add additional tests concerning the use of aliases for option groups
  in config files. [#666][]
- Build: Add support for testing in meson and cleanup symbolic link generation.
  [#701][], [#697][]
- Build: Support building in WebAssembly. [#679][]

[#660]: https://github.com/CLIUtils/CLI11/pull/660
[#661]: https://github.com/CLIUtils/CLI11/pull/661
[#663]: https://github.com/CLIUtils/CLI11/pull/663
[#665]: https://github.com/CLIUtils/CLI11/pull/665
[#666]: https://github.com/CLIUtils/CLI11/pull/666
[#668]: https://github.com/CLIUtils/CLI11/pull/668
[#678]: https://github.com/CLIUtils/CLI11/pull/678
[#679]: https://github.com/CLIUtils/CLI11/pull/679
[#690]: https://github.com/CLIUtils/CLI11/pull/690
[#697]: https://github.com/CLIUtils/CLI11/pull/697
[#698]: https://github.com/CLIUtils/CLI11/pull/698
[#701]: https://github.com/CLIUtils/CLI11/pull/701
[#706]: https://github.com/CLIUtils/CLI11/pull/706
[#707]: https://github.com/CLIUtils/CLI11/pull/707
[#709]: https://github.com/CLIUtils/CLI11/pull/709
[#713]: https://github.com/CLIUtils/CLI11/pull/713

## Version 2.1: Names and callbacks

The name restrictions for options and subcommands are now much looser, allowing
a wider variety of characters than before, even spaces can be used (use quotes
to include a space in most shells). The default configuration parser was
improved, allowing your configuration to sit in a larger file. And option
callbacks have a few new settings, allowing them to be run even if the option is
not passed, or every time the option is parsed.

- Option/subcommand name restrictions have been relaxed. Most characters are now
  allowed. [#627][]
- The config parser can accept streams, specify a specific section, and inline
  comment characters are supported [#630][]
- `force_callback` & `trigger_on_parse` added, allowing a callback to always run
  on parse even if not present or every time the option is parsed [#631][]
- Bugfix(cmake): Only add `CONFIGURE_DEPENDS` if CLI11 is the main project
  [#633][]
- Bugfix(cmake): Ensure the cmake/pkg-config files install to a arch independent
  path [#635][]
- Bugfix: The single header file generation was missing the include guard.
  [#620][]

[#620]: https://github.com/CLIUtils/CLI11/pull/620
[#627]: https://github.com/CLIUtils/CLI11/pull/627
[#630]: https://github.com/CLIUtils/CLI11/pull/630
[#631]: https://github.com/CLIUtils/CLI11/pull/631
[#633]: https://github.com/CLIUtils/CLI11/pull/633
[#635]: https://github.com/CLIUtils/CLI11/pull/635

### Version 2.1.1: Quick Windows fix

- A collision with `min`/`max` macros on Windows has been fixed. [#642][]
- Tests pass with Boost again [#646][]
- Running the pre-commit hooks in development no longer requires docker for
  clang-format [#647][]

[#642]: https://github.com/CLIUtils/CLI11/pull/642
[#646]: https://github.com/CLIUtils/CLI11/pull/646
[#647]: https://github.com/CLIUtils/CLI11/pull/647

## Version 2.1.2: Better subproject builds

- Use `main` for the main branch of the repository [#657][]
- Bugfix(cmake): Enforce at least C++11 when using CMake target [#656][]
- Build: Don't run doxygen and CTest includes if a submodule [#656][]
- Build: Avoid a warning on CMake 3.22 [#656][]
- Build: Support compiling the tests with an external copy of Catch2 [#653][]

[#653]: https://github.com/CLIUtils/CLI11/pull/653
[#656]: https://github.com/CLIUtils/CLI11/pull/656
[#657]: https://github.com/CLIUtils/CLI11/pull/657

## Version 2.0: Simplification

This version focuses on cleaning up deprecated functionality, and some minor
default changes. The config processing is TOML compliant now. Atomics and
complex numbers are directly supported, along with other container improvements.
A new version flag option has finally been added. Subcommands are significantly
improved with new features and bugfixes for corner cases. This release contains
a lot of backend cleanup, including a complete overhaul of the testing system
and single file generation system.

- Built-in config format is TOML compliant now [#435][]
  - Support multiline TOML [#528][]
  - Support for configurable quotes [#599][]
  - Support short/positional options in config mode [#443][]
- More powerful containers, support for `%%` separator [#423][]
- Support atomic types [#520][] and complex types natively [#423][]
- Add a type validator `CLI::TypeValidator<TYPE>` [#526][]
- Add a version flag easily [#452][], with help message [#601][]
- Support `->silent()` on subcommands. [#529][]
- Add alias section to help for subcommands [#545][]
- Allow quotes to specify a program name [#605][]
- Backend: redesigned MakeSingleFiles to have a higher level of manual control,
  to support future features. [#546][]
- Backend: moved testing from GTest to Catch2 [#574][]
- Bugfix: avoid duplicated and missed calls to the final callback [#584][]
- Bugfix: support embedded newlines in more places [#592][]
- Bugfix: avoid listing helpall as a required flag [#530][]
- Bugfix: avoid a clash with WINDOWS define [#563][]
- Bugfix: the help flag didn't get processed when a config file was required
  [#606][]
- Bugfix: fix description of non-configurable subcommands in config [#604][]
- Build: support pkg-config [#523][]

> ### Converting from CLI11 1.9
>
> - Removed deprecated set commands, use validators instead. [#565][]
> - The final "defaulted" bool has been removed, use `->capture_default_str()`
>   instead. Use `app.option_defaults()->always_capture_default()` to set this
>   for all future options. [#597][]
> - Use `add_option` on a complex number instead of `add_complex`, which has
>   been removed.

[#423]: https://github.com/CLIUtils/CLI11/pull/423
[#435]: https://github.com/CLIUtils/CLI11/pull/435
[#443]: https://github.com/CLIUtils/CLI11/pull/443
[#452]: https://github.com/CLIUtils/CLI11/pull/452
[#520]: https://github.com/CLIUtils/CLI11/pull/520
[#523]: https://github.com/CLIUtils/CLI11/pull/523
[#526]: https://github.com/CLIUtils/CLI11/pull/526
[#528]: https://github.com/CLIUtils/CLI11/pull/528
[#529]: https://github.com/CLIUtils/CLI11/pull/529
[#530]: https://github.com/CLIUtils/CLI11/pull/530
[#545]: https://github.com/CLIUtils/CLI11/pull/545
[#546]: https://github.com/CLIUtils/CLI11/pull/546
[#563]: https://github.com/CLIUtils/CLI11/pull/563
[#565]: https://github.com/CLIUtils/CLI11/pull/565
[#574]: https://github.com/CLIUtils/CLI11/pull/574
[#584]: https://github.com/CLIUtils/CLI11/pull/584
[#592]: https://github.com/CLIUtils/CLI11/pull/592
[#597]: https://github.com/CLIUtils/CLI11/pull/597
[#599]: https://github.com/CLIUtils/CLI11/pull/599
[#601]: https://github.com/CLIUtils/CLI11/pull/601
[#604]: https://github.com/CLIUtils/CLI11/pull/604
[#605]: https://github.com/CLIUtils/CLI11/pull/605
[#606]: https://github.com/CLIUtils/CLI11/pull/606

## Version 1.9: Config files and cleanup

Config file handling was revamped to fix common issues, and now supports reading
[TOML](https://github.com/toml-lang/toml).

Adding options is significantly more powerful with support for things like
`std::tuple` and `std::array`, including with transforms. Several new
configuration options were added to facilitate a wider variety of apps. GCC 4.7
is no longer supported.

- Config files refactored, supports TOML (may become default output in 2.0)
  [#362][]
- Added two template parameter form of `add_option`, allowing `std::optional` to
  be supported without a special import [#285][]
- `string_view` now supported in reasonable places [#300][], [#285][]
- `immediate_callback`, `final_callback`, and `parse_complete_callback` added to
  support controlling the App callback order [#292][], [#313][]
- Multiple positional arguments maintain order if `positionals_at_end` is set.
  [#306][]
- Pair/tuple/array now supported, and validators indexed to specific components
  in the objects [#307][], [#310][]
- Footer callbacks supported [#309][]
- Subcommands now support needs (including nameless subcommands) [#317][]
- More flexible type size, more useful `add_complex` [#325][], [#370][]
- Added new validators `CLI::NonNegativeNumber` and `CLI::PositiveNumber`
  [#342][]
- Transform now supports arrays [#349][]
- Option groups can be hidden [#356][]
- Add `CLI::deprecate_option` and `CLI::retire_option` functions [#358][]
- More flexible and safer Option `default_val` [#387][]
- Backend: Cleaner type traits [#286][]
- Backend: File checking updates [#341][]
- Backend: Using pre-commit to format, checked in GitHub Actions [#336][]
- Backend: Clang-tidy checked again, CMake option now `CL11_CLANG_TIDY` [#390][]
- Backend: Warning cleanup, more checks from klocwork [#350][], Effective C++
  [#354][], clang-tidy [#360][], CUDA NVCC [#365][], cross compile [#373][],
  sign conversion [#382][], and cpplint [#400][]
- Docs: CLI11 Tutorial now hosted in the same repository [#304][], [#318][],
  [#374][]
- Bugfix: Fixed undefined behavior in `checked_multiply` [#290][]
- Bugfix: `->check()` was adding the name to the wrong validator [#320][]
- Bugfix: Resetting config option works properly [#301][]
- Bugfix: Hidden flags were showing up in error printout [#333][]
- Bugfix: Enum conversion no longer broken if stream operator added [#348][]
- Build: The meson build system supported [#299][]
- Build: GCC 4.7 is no longer supported, due mostly to GoogleTest. GCC 4.8+ is
  now required. [#160][]
- Build: Restructured significant portions of CMake build system [#394][]

> ### Converting from CLI11 1.8
>
> - Some deprecated methods dropped
>   - `add_set*` should be replaced with `->check`/`->transform` and
>     `CLI::IsMember` since 1.8
>   - `get_defaultval` was replaced by `get_default_str` in 1.8
> - The true/false 4th argument to `add_option` is expected to be removed in
>   2.0, use `->capture_default_str()` since 1.8

[#160]: https://github.com/CLIUtils/CLI11/pull/160
[#285]: https://github.com/CLIUtils/CLI11/pull/285
[#286]: https://github.com/CLIUtils/CLI11/pull/286
[#290]: https://github.com/CLIUtils/CLI11/pull/290
[#292]: https://github.com/CLIUtils/CLI11/pull/292
[#299]: https://github.com/CLIUtils/CLI11/pull/299
[#300]: https://github.com/CLIUtils/CLI11/pull/300
[#301]: https://github.com/CLIUtils/CLI11/pull/301
[#304]: https://github.com/CLIUtils/CLI11/pull/304
[#306]: https://github.com/CLIUtils/CLI11/pull/306
[#307]: https://github.com/CLIUtils/CLI11/pull/307
[#309]: https://github.com/CLIUtils/CLI11/pull/309
[#310]: https://github.com/CLIUtils/CLI11/pull/310
[#313]: https://github.com/CLIUtils/CLI11/pull/313
[#317]: https://github.com/CLIUtils/CLI11/pull/317
[#318]: https://github.com/CLIUtils/CLI11/pull/318
[#320]: https://github.com/CLIUtils/CLI11/pull/320
[#325]: https://github.com/CLIUtils/CLI11/pull/325
[#333]: https://github.com/CLIUtils/CLI11/pull/333
[#336]: https://github.com/CLIUtils/CLI11/pull/336
[#341]: https://github.com/CLIUtils/CLI11/pull/341
[#342]: https://github.com/CLIUtils/CLI11/pull/342
[#348]: https://github.com/CLIUtils/CLI11/pull/348
[#349]: https://github.com/CLIUtils/CLI11/pull/349
[#350]: https://github.com/CLIUtils/CLI11/pull/350
[#354]: https://github.com/CLIUtils/CLI11/pull/354
[#356]: https://github.com/CLIUtils/CLI11/pull/356
[#358]: https://github.com/CLIUtils/CLI11/pull/358
[#360]: https://github.com/CLIUtils/CLI11/pull/360
[#362]: https://github.com/CLIUtils/CLI11/pull/362
[#365]: https://github.com/CLIUtils/CLI11/pull/365
[#370]: https://github.com/CLIUtils/CLI11/pull/370
[#373]: https://github.com/CLIUtils/CLI11/pull/373
[#374]: https://github.com/CLIUtils/CLI11/pull/374
[#382]: https://github.com/CLIUtils/CLI11/pull/382
[#387]: https://github.com/CLIUtils/CLI11/pull/387
[#390]: https://github.com/CLIUtils/CLI11/pull/390
[#394]: https://github.com/CLIUtils/CLI11/pull/394
[#400]: https://github.com/CLIUtils/CLI11/pull/400

### Version 1.9.1: Backporting fixes

This is a patch version that backports fixes from the development of 2.0.

- Support relative inclusion [#475][]
- Fix cases where spaces in paths could break CMake support [#471][]
- Fix an issue with string conversion [#421][]
- Cross-compiling improvement for Conan.io [#430][]
- Fix option group default propagation [#450][]
- Fix for C++20 [#459][]
- Support compiling with RTTI off [#461][]

[#421]: https://github.com/CLIUtils/CLI11/pull/421
[#430]: https://github.com/CLIUtils/CLI11/pull/430
[#450]: https://github.com/CLIUtils/CLI11/pull/450
[#459]: https://github.com/CLIUtils/CLI11/pull/459
[#461]: https://github.com/CLIUtils/CLI11/pull/461
[#471]: https://github.com/CLIUtils/CLI11/pull/471
[#475]: https://github.com/CLIUtils/CLI11/pull/475

## Version 1.8: Transformers, default strings, and flags

Set handling has been completely replaced by a new backend that works as a
Validator or Transformer. This provides a single interface instead of the 16
different functions in App. It also allows ordered collections to be used,
custom functions for filtering, and better help and error messages. You can also
use a collection of pairs (like `std::map`) to transform the match into an
output. Also new are inverted flags, which can cancel or reduce the count of
flags, and can also support general flag types. A new `add_option_fn` lets you
more easily program CLI11 options with the types you choose. Vector options now
support a custom separator. Apps can now be composed with unnamed subcommand
support. The final bool "defaults" flag when creating options has been replaced
by `->capture_default_str()` (ending an old limitation in construction made this
possible); the old method is still available but may be removed in future
versions.

- Replaced default help capture: `.add_option("name", value, "", True)` becomes
  `.add_option("name", value)->capture_default_str()` [#242][]
- Added `.always_capture_default()` [#242][]
- New `CLI::IsMember` validator replaces set validation [#222][]
- `IsMember` also supports container of pairs, transform allows modification of
  result [#228][]
- Added new Transformers, `CLI::AsNumberWithUnit` and `CLI::AsSizeValue`
  [#253][]
- Much more powerful flags with different values [#211][], general types
  [#235][]
- `add_option` now supports bool due to unified bool handling [#211][]
- Support for composable unnamed subcommands [#216][]
- Reparsing is better supported with `.remaining_for_passthrough()` [#265][]
- Custom vector separator using `->delimiter(char)` [#209][], [#221][], [#240][]
- Validators added for IP4 addresses and positive numbers [#210][] and numbers
  [#262][]
- Minimum required Boost for optional Optionals has been corrected to 1.61
  [#226][]
- Positionals can stop options from being parsed with `app.positionals_at_end()`
  [#223][]
- Added `validate_positionals` [#262][]
- Positional parsing is much more powerful [#251][], duplicates supported
  [#247][]
- Validators can be negated with `!` [#230][], and now handle tname functions
  [#228][]
- Better enum support and streaming helper [#233][] and [#228][]
- Cleanup for shadow warnings [#232][]
- Better alignment on multiline descriptions [#269][]
- Better support for aarch64 [#266][]
- Respect `BUILD_TESTING` only if CLI11 is the main project; otherwise,
  `CLI11_TESTING` must be used [#277][]
- Drop auto-detection of experimental optional and boost::optional; must be
  enabled explicitly (too fragile) [#277][] [#279][]

> ### Converting from CLI11 1.7
>
> - `.add_option(..., true)` should be replaced by
>   `.add_option(...)->capture_default_str()` or
>   `app.option_defaults()->always_capture_default()` can be used
> - `app.add_set("--name", value, {"choice1", "choice2"})` should become
>   `app.add_option("--name", value)->check(CLI::IsMember({"choice1", "choice2"}))`
> - The `_ignore_case` version of this can be replaced by adding
>   `CLI::ignore_case` to the argument list in `IsMember`
> - The `_ignore_underscore` version of this can be replaced by adding
>   `CLI::ignore_underscore` to the argument list in `IsMember`
> - The `_ignore_case_underscore` version of this can be replaced by adding both
>   functions listed above to the argument list in `IsMember`
> - If you want an exact match to the original choice after one of the modifier
>   functions matches, use `->transform` instead of `->check`
> - The `_mutable` versions of this can be replaced by passing a pointer or
>   shared pointer into `IsMember`
> - An error with sets now produces a `ValidationError` instead of a
>   `ConversionError`

[#209]: https://github.com/CLIUtils/CLI11/pull/209
[#210]: https://github.com/CLIUtils/CLI11/pull/210
[#211]: https://github.com/CLIUtils/CLI11/pull/211
[#216]: https://github.com/CLIUtils/CLI11/pull/216
[#221]: https://github.com/CLIUtils/CLI11/pull/221
[#222]: https://github.com/CLIUtils/CLI11/pull/222
[#223]: https://github.com/CLIUtils/CLI11/pull/223
[#226]: https://github.com/CLIUtils/CLI11/pull/226
[#228]: https://github.com/CLIUtils/CLI11/pull/228
[#230]: https://github.com/CLIUtils/CLI11/pull/230
[#232]: https://github.com/CLIUtils/CLI11/pull/232
[#233]: https://github.com/CLIUtils/CLI11/pull/233
[#235]: https://github.com/CLIUtils/CLI11/pull/235
[#240]: https://github.com/CLIUtils/CLI11/pull/240
[#242]: https://github.com/CLIUtils/CLI11/pull/242
[#247]: https://github.com/CLIUtils/CLI11/pull/247
[#251]: https://github.com/CLIUtils/CLI11/pull/251
[#253]: https://github.com/CLIUtils/CLI11/pull/253
[#262]: https://github.com/CLIUtils/CLI11/pull/262
[#265]: https://github.com/CLIUtils/CLI11/pull/265
[#266]: https://github.com/CLIUtils/CLI11/pull/266
[#269]: https://github.com/CLIUtils/CLI11/pull/269
[#277]: https://github.com/CLIUtils/CLI11/pull/277
[#279]: https://github.com/CLIUtils/CLI11/pull/279

## Version 2.1: Names and callbacks

The name restrictions for options and subcommands are now much looser, allowing
a wider variety of characters than before, even spaces can be used (use quotes
to include a space in most shells). The default configuration parser was
improved, allowing your configuration to sit in a larger file. And option
callbacks have a few new settings, allowing them to be run even if the option is
not passed, or every time the option is parsed.

- Option/subcommand name restrictions have been relaxed. Most characters are now
  allowed. [#627][]
- The config parser can accept streams, specify a specific section, and inline
  comment characters are supported [#630][]
- `force_callback` & `trigger_on_parse` added, allowing a callback to always run
  on parse even if not present or every time the option is parsed [#631][]
- Bugfix(cmake): Only add `CONFIGURE_DEPENDS` if CLI11 is the main project
  [#633][]
- Bugfix(cmake): Ensure the cmake/pkg-config files install to a arch independent
  path [#635][]
- Bugfix: The single header file generation was missing the include guard.
  [#620][]

[#620]: https://github.com/CLIUtils/CLI11/pull/620
[#627]: https://github.com/CLIUtils/CLI11/pull/627
[#630]: https://github.com/CLIUtils/CLI11/pull/630
[#631]: https://github.com/CLIUtils/CLI11/pull/631
[#633]: https://github.com/CLIUtils/CLI11/pull/633
[#635]: https://github.com/CLIUtils/CLI11/pull/635

### Version 2.1.1: Quick Windows fix

- A collision with `min`/`max` macros on Windows has been fixed. [#642][]
- Tests pass with Boost again [#646][]
- Running the pre-commit hooks in development no longer requires docker for
  clang-format [#647][]

[#642]: https://github.com/CLIUtils/CLI11/pull/642
[#646]: https://github.com/CLIUtils/CLI11/pull/646
[#647]: https://github.com/CLIUtils/CLI11/pull/647

## Version 2.1.2: Better subproject builds

- Use `main` for the main branch of the repository [#657][]
- Bugfix(cmake): Enforce at least C++11 when using CMake target [#656][]
- Build: Don't run doxygen and CTest includes if a submodule [#656][]
- Build: Avoid a warning on CMake 3.22 [#656][]
- Build: Support compiling the tests with an external copy of Catch2 [#653][]

[#653]: https://github.com/CLIUtils/CLI11/pull/653
[#656]: https://github.com/CLIUtils/CLI11/pull/656
[#657]: https://github.com/CLIUtils/CLI11/pull/657

## Version 2.0: Simplification

This version focuses on cleaning up deprecated functionality, and some minor
default changes. The config processing is TOML compliant now. Atomics and
complex numbers are directly supported, along with other container improvements.
A new version flag option has finally been added. Subcommands are significantly
improved with new features and bugfixes for corner cases. This release contains
a lot of backend cleanup, including a complete overhaul of the testing system
and single file generation system.

- Built-in config format is TOML compliant now [#435][]
  - Support multiline TOML [#528][]
  - Support for configurable quotes [#599][]
  - Support short/positional options in config mode [#443][]
- More powerful containers, support for `%%` separator [#423][]
- Support atomic types [#520][] and complex types natively [#423][]
- Add a type validator `CLI::TypeValidator<TYPE>` [#526][]
- Add a version flag easily [#452][], with help message [#601][]
- Support `->silent()` on subcommands. [#529][]
- Add alias section to help for subcommands [#545][]
- Allow quotes to specify a program name [#605][]
- Backend: redesigned MakeSingleFiles to have a higher level of manual control,
  to support future features. [#546][]
- Backend: moved testing from GTest to Catch2 [#574][]
- Bugfix: avoid duplicated and missed calls to the final callback [#584][]
- Bugfix: support embedded newlines in more places [#592][]
- Bugfix: avoid listing helpall as a required flag [#530][]
- Bugfix: avoid a clash with WINDOWS define [#563][]
- Bugfix: the help flag didn't get processed when a config file was required
  [#606][]
- Bugfix: fix description of non-configurable subcommands in config [#604][]
- Build: support pkg-config [#523][]

> ### Converting from CLI11 1.9
>
> - Removed deprecated set commands, use validators instead. [#565][]
> - The final "defaulted" bool has been removed, use `->capture_default_str()`
>   instead. Use `app.option_defaults()->always_capture_default()` to set this
>   for all future options. [#597][]
> - Use `add_option` on a complex number instead of `add_complex`, which has
>   been removed.

[#423]: https://github.com/CLIUtils/CLI11/pull/423
[#435]: https://github.com/CLIUtils/CLI11/pull/435
[#443]: https://github.com/CLIUtils/CLI11/pull/443
[#452]: https://github.com/CLIUtils/CLI11/pull/452
[#520]: https://github.com/CLIUtils/CLI11/pull/520
[#523]: https://github.com/CLIUtils/CLI11/pull/523
[#526]: https://github.com/CLIUtils/CLI11/pull/526
[#528]: https://github.com/CLIUtils/CLI11/pull/528
[#529]: https://github.com/CLIUtils/CLI11/pull/529
[#530]: https://github.com/CLIUtils/CLI11/pull/530
[#545]: https://github.com/CLIUtils/CLI11/pull/545
[#546]: https://github.com/CLIUtils/CLI11/pull/546
[#563]: https://github.com/CLIUtils/CLI11/pull/563
[#565]: https://github.com/CLIUtils/CLI11/pull/565
[#574]: https://github.com/CLIUtils/CLI11/pull/574
[#584]: https://github.com/CLIUtils/CLI11/pull/584
[#592]: https://github.com/CLIUtils/CLI11/pull/592
[#597]: https://github.com/CLIUtils/CLI11/pull/597
[#599]: https://github.com/CLIUtils/CLI11/pull/599
[#601]: https://github.com/CLIUtils/CLI11/pull/601
[#604]: https://github.com/CLIUtils/CLI11/pull/604
[#605]: https://github.com/CLIUtils/CLI11/pull/605
[#606]: https://github.com/CLIUtils/CLI11/pull/606

## Version 1.9: Config files and cleanup

Config file handling was revamped to fix common issues, and now supports reading
[TOML](https://github.com/toml-lang/toml).

Adding options is significantly more powerful with support for things like
`std::tuple` and `std::array`, including with transforms. Several new
configuration options were added to facilitate a wider variety of apps. GCC 4.7
is no longer supported.

- Config files refactored, supports TOML (may become default output in 2.0)
  [#362][]
- Added two template parameter form of `add_option`, allowing `std::optional` to
  be supported without a special import [#285][]
- `string_view` now supported in reasonable places [#300][], [#285][]
- `immediate_callback`, `final_callback`, and `parse_complete_callback` added to
  support controlling the App callback order [#292][], [#313][]
- Multiple positional arguments maintain order if `positionals_at_end` is set.
  [#306][]
- Pair/tuple/array now supported, and validators indexed to specific components
  in the objects [#307][], [#310][]
- Footer callbacks supported [#309][]
- Subcommands now support needs (including nameless subcommands) [#317][]
- More flexible type size, more useful `add_complex` [#325][], [#370][]
- Added new validators `CLI::NonNegativeNumber` and `CLI::PositiveNumber`
  [#342][]
- Transform now supports arrays [#349][]
- Option groups can be hidden [#356][]
- Add `CLI::deprecate_option` and `CLI::retire_option` functions [#358][]
- More flexible and safer Option `default_val` [#387][]
- Backend: Cleaner type traits [#286][]
- Backend: File checking updates [#341][]
- Backend: Using pre-commit to format, checked in GitHub Actions [#336][]
- Backend: Clang-tidy checked again, CMake option now `CL11_CLANG_TIDY` [#390][]
- Backend: Warning cleanup, more checks from klocwork [#350][], Effective C++
  [#354][], clang-tidy [#360][], CUDA NVCC [#365][], cross compile [#373][],
  sign conversion [#382][], and cpplint [#400][]
- Docs: CLI11 Tutorial now hosted in the same repository [#304][], [#318][],
  [#374][]
- Bugfix: Fixed undefined behavior in `checked_multiply` [#290][]
- Bugfix: `->check()` was adding the name to the wrong validator [#320][]
- Bugfix: Resetting config option works properly [#301][]
- Bugfix: Hidden flags were showing up in error printout [#333][]
- Bugfix: Enum conversion no longer broken if stream operator added [#348][]
- Build: The meson build system supported [#299][]
- Build: GCC 4.7 is no longer supported, due mostly to GoogleTest. GCC 4.8+ is
  now required. [#160][]
- Build: Restructured significant portions of CMake build system [#394][]

> ### Converting from CLI11 1.8
>
> - Some deprecated methods dropped
>   - `add_set*` should be replaced with `->check`/`->transform` and
>     `CLI::IsMember` since 1.8
>   - `get_defaultval` was replaced by `get_default_str` in 1.8
> - The true/false 4th argument to `add_option` is expected to be removed in
>   2.0, use `->capture_default_str()` since 1.8

[#160]: https://github.com/CLIUtils/CLI11/pull/160
[#285]: https://github.com/CLIUtils/CLI11/pull/285
[#286]: https://github.com/CLIUtils/CLI11/pull/286
[#290]: https://github.com/CLIUtils/CLI11/pull/290
[#292]: https://github.com/CLIUtils/CLI11/pull/292
[#299]: https://github.com/CLIUtils/CLI11/pull/299
[#300]: https://github.com/CLIUtils/CLI11/pull/300
[#301]: https://github.com/CLIUtils/CLI11/pull/301
[#304]: https://github.com/CLIUtils/CLI11/pull/304
[#306]: https://github.com/CLIUtils/CLI11/pull/306
[#307]: https://github.com/CLIUtils/CLI11/pull/307
[#309]: https://github.com/CLIUtils/CLI11/pull/309
[#310]: https://github.com/CLIUtils/CLI11/pull/310
[#313]: https://github.com/CLIUtils/CLI11/pull/313
[#317]: https://github.com/CLIUtils/CLI11/pull/317
[#318]: https://github.com/CLIUtils/CLI11/pull/318
[#320]: https://github.com/CLIUtils/CLI11/pull/320
[#325]: https://github.com/CLIUtils/CLI11/pull/325
[#333]: https://github.com/CLIUtils/CLI11/pull/333
[#336]: https://github.com/CLIUtils/CLI11/pull/336
[#341]: https://github.com/CLIUtils/CLI11/pull/341
[#342]: https://github.com/CLIUtils/CLI11/pull/342
[#348]: https://github.com/CLIUtils/CLI11/pull/348
[#349]: https://github.com/CLIUtils/CLI11/pull/349
[#350]: https://github.com/CLIUtils/CLI11/pull/350
[#354]: https://github.com/CLIUtils/CLI11/pull/354
[#356]: https://github.com/CLIUtils/CLI11/pull/356
[#358]: https://github.com/CLIUtils/CLI11/pull/358
[#360]: https://github.com/CLIUtils/CLI11/pull/360
[#362]: https://github.com/CLIUtils/CLI11/pull/362
[#365]: https://github.com/CLIUtils/CLI11/pull/365
[#370]: https://github.com/CLIUtils/CLI11/pull/370
[#373]: https://github.com/CLIUtils/CLI11/pull/373
[#374]: https://github.com/CLIUtils/CLI11/pull/374
[#382]: https://github.com/CLIUtils/CLI11/pull/382
[#387]: https://github.com/CLIUtils/CLI11/pull/387
[#390]: https://github.com/CLIUtils/CLI11/pull/390
[#394]: https://github.com/CLIUtils/CLI11/pull/394
[#400]: https://github.com/CLIUtils/CLI11/pull/400

### Version 1.9.1: Backporting fixes

This is a patch version that backports fixes from the development of 2.0.

- Support relative inclusion [#475][]
- Fix cases where spaces in paths could break CMake support [#471][]
- Fix an issue with string conversion [#421][]
- Cross-compiling improvement for Conan.io [#430][]
- Fix option group default propagation [#450][]
- Fix for C++20 [#459][]
- Support compiling with RTTI off [#461][]

[#421]: https://github.com/CLIUtils/CLI11/pull/421
[#430]: https://github.com/CLIUtils/CLI11/pull/430
[#450]: https://github.com/CLIUtils/CLI11/pull/450
[#459]: https://github.com/CLIUtils/CLI11/pull/459
[#461]: https://github.com/CLIUtils/CLI11/pull/461
[#471]: https://github.com/CLIUtils/CLI11/pull/471
[#475]: https://github.com/CLIUtils/CLI11/pull/475

## Version 1.8: Transformers, default strings, and flags

Set handling has been completely replaced by a new backend that works as a
Validator or Transformer. This provides a single interface instead of the 16
different functions in App. It also allows ordered collections to be used,
custom functions for filtering, and better help and error messages. You can also
use a collection of pairs (like `std::map`) to transform the match into an
output. Also new are inverted flags, which can cancel or reduce the count of
flags, and can also support general flag types. A new `add_option_fn` lets you
more easily program CLI11 options with the types you choose. Vector options now
support a custom separator. Apps can now be composed with unnamed subcommand
support. The final bool "defaults" flag when creating options has been replaced
by `->capture_default_str()` (ending an old limitation in construction made this
possible); the old method is still available but may be removed in future
versions.

- Replaced default help capture: `.add_option("name", value, "", True)` becomes
  `.add_option("name", value)->capture_default_str()` [#242][]
- Added `.always_capture_default()` [#242][]
- New `CLI::IsMember` validator replaces set validation [#222][]
- `IsMember` also supports container of pairs, transform allows modification of
  result [#228][]
- Added new Transformers, `CLI::AsNumberWithUnit` and `CLI::AsSizeValue`
  [#253][]
- Much more powerful flags with different values [#211][], general types
  [#235][]
- `add_option` now supports bool due to unified bool handling [#211][]
- Support for composable unnamed subcommands [#216][]
- Reparsing is better supported with `.remaining_for_passthrough()` [#265][]
- Custom vector separator using `->delimiter(char)` [#209][], [#221][], [#240][]
- Validators added for IP4 addresses and positive numbers [#210][] and numbers
  [#262][]
- Minimum required Boost for optional Optionals has been corrected to 1.61
  [#226][]
- Positionals can stop options from being parsed with `app.positionals_at_end()`
  [#223][]
- Added `validate_positionals` [#262][]
- Positional parsing is much more powerful [#251][], duplicates supported
  [#247][]
- Validators can be negated with `!` [#230][], and now handle tname functions
  [#228][]
- Better enum support and streaming helper [#233][] and [#228][]
- Cleanup for shadow warnings [#232][]
- Better alignment on multiline descriptions [#269][]
- Better support for aarch64 [#266][]
- Respect `BUILD_TESTING` only if CLI11 is the main project; otherwise,
  `CLI11_TESTING` must be used [#277][]
- Drop auto-detection of experimental optional and boost::optional; must be
  enabled explicitly (too fragile) [#277][] [#279][]

> ### Converting from CLI11 1.7
>
> - `.add_option(..., true)` should be replaced by
>   `.add_option(...)->capture_default_str()` or
>   `app.option_defaults()->always_capture_default()` can be used
> - `app.add_set("--name", value, {"choice1", "choice2"})` should become
>   `app.add_option("--name", value)->check(CLI::IsMember({"choice1", "choice2"}))`
> - The `_ignore_case` version of this can be replaced by adding
>   `CLI::ignore_case` to the argument list in `IsMember`
> - The `_ignore_underscore` version of this can be replaced by adding
>   `CLI::ignore_underscore` to the argument list in `IsMember`
> - The `_ignore_case_underscore` version of this can be replaced by adding both
>   functions listed above to the argument list in `IsMember`
> - If you want an exact match to the original choice after one of the modifier
>   functions matches, use `->transform` instead of `->check`
> - The `_mutable` versions of this can be replaced by passing a pointer or
>   shared pointer into `IsMember`
> - An error with sets now produces a `ValidationError` instead of a
>   `ConversionError`

[#209]: https://github.com/CLIUtils/CLI11/pull/209
[#210]: https://github.com/CLIUtils/CLI11/pull/210
[#211]: https://github.com/CLIUtils/CLI11/pull/211
[#216]: https://github.com/CLIUtils/CLI11/pull/216
[#221]: https://github.com/CLIUtils/CLI11/pull/221
[#222]: https://github.com/CLIUtils/CLI11/pull/222
[#223]: https://github.com/CLIUtils/CLI11/pull/223
[#226]: https://github.com/CLIUtils/CLI11/pull/226
[#228]: https://github.com/CLIUtils/CLI11/pull/228
[#230]: https://github.com/CLIUtils/CLI11/pull/230
[#232]: https://github.com/CLIUtils/CLI11/pull/232
[#233]: https://github.com/CLIUtils/CLI11/pull/233
[#235]: https://github.com/CLIUtils/CLI11/pull/235
[#240]: https://github.com/CLIUtils/CLI11/pull/240
[#242]: https://github.com/CLIUtils/CLI11/pull/242
[#247]: https://github.com/CLIUtils/CLI11/pull/247
[#251]: https://github.com/CLIUtils/CLI11/pull/251
[#253]: https://github.com/CLIUtils/CLI11/pull/253
[#262]: https://github.com/CLIUtils/CLI11/pull/262
[#265]: https://github.com/CLIUtils/CLI11/pull/265
[#266]: https://github.com/CLIUtils/CLI11/pull/266
[#269]: https://github.com/CLIUtils/CLI11/pull/269
[#277]: https://github.com/CLIUtils/CLI11/pull/277
[#279]: https://github.com/CLIUtils/CLI11/pull/279

## Version 2.1: Names and callbacks

The name restrictions for options and subcommands are now much looser, allowing
a wider variety of characters than before, even spaces can be used (use quotes
to include a space in most shells). The default configuration parser was
improved, allowing your configuration to sit in a larger file. And option
callbacks have a few new settings, allowing them to be run even if the option is
not passed, or every time the option is parsed.

- Option/subcommand name restrictions have been relaxed. Most characters are now
  allowed. [#627][]
- The config parser can accept streams, specify a specific section, and inline
  comment characters are supported [#630][]
- `force_callback` & `trigger_on_parse` added, allowing a callback to always run
  on parse even if not present or every time the option is parsed [#631][]
- Bugfix(cmake): Only add `CONFIGURE_DEPENDS` if CLI11 is the main project
  [#633][]
- Bugfix(cmake): Ensure the cmake/pkg-config files install to a arch independent
  path [#635][]
- Bugfix: The single header file generation was missing the include guard.
  [#620][]

[#620]: https://github.com/CLIUtils/CLI11/pull/620
[#627]: https://github.com/CLIUtils/CLI11/pull/627
[#630]: https://github.com/CLIUtils/CLI11/pull/630
[#631]: https://github.com/CLIUtils/CLI11/pull/631
[#633]: https://github.com/CLIUtils/CLI11/pull/633
[#635]: https://github.com/CLIUtils/CLI11/pull/635

### Version 2.1.1: Quick Windows fix

- A collision with `min`/`max` macros on Windows has been fixed. [#642][]
- Tests pass with Boost again [#646][]
- Running the pre-commit hooks in development no longer requires docker for
  clang-format [#647][]

[#642]: https://github.com/CLIUtils/CLI11/pull/642
[#646]: https://github.com/CLIUtils/CLI11/pull/646
[#647]: https://github.com/CLIUtils/CLI11/pull/647

## Version 2.1.2: Better subproject builds

- Use `main` for the main branch of the repository [#657][]
- Bugfix(cmake): Enforce at least C++11 when using CMake target [#656][]
- Build: Don't run doxygen and CTest includes if a submodule [#656][]
- Build: Avoid a warning on CMake 3.22 [#656][]
- Build: Support compiling the tests with an external copy of Catch2 [#653][]

[#653]: https://github.com/CLIUtils/CLI11/pull/653
[#656]: https://github.com/CLIUtils/CLI11/pull/656
[#657]: https://github.com/CLIUtils/CLI11/pull/657

## Version 2.0: Simplification

This version focuses on cleaning up deprecated functionality, and some minor
default changes. The config processing is TOML compliant now. Atomics and
complex numbers are directly supported, along with other container improvements.
A new version flag option has finally been added. Subcommands are significantly
improved with new features and bugfixes for corner cases. This release contains
a lot of backend cleanup, including a complete overhaul of the testing system
and single file generation system.

- Built-in config format is TOML compliant now [#435][]
  - Support multiline TOML [#528][]
  - Support for configurable quotes [#599][]
  - Support short/positional options in config mode [#443][]
- More powerful containers, support for `%%` separator [#423][]
- Support atomic types [#520][] and complex types natively [#423][]
- Add a type validator `CLI::TypeValidator<TYPE>` [#526][]
- Add a version flag easily [#452][], with help message [#601][]
- Support `->silent()` on subcommands. [#529][]
- Add alias section to help for subcommands [#545][]
- Allow quotes to specify a program name [#605][]
- Backend: redesigned MakeSingleFiles to have a higher level of manual control,
  to support future features. [#546][]
- Backend: moved testing from GTest to Catch2 [#574][]
- Bugfix: avoid duplicated and missed calls to the final callback [#584][]
- Bugfix: support embedded newlines in more places [#592][]
- Bugfix: avoid listing helpall as a required flag [#530][]
- Bugfix: avoid a clash with WINDOWS define [#563][]
- Bugfix: the help flag didn't get processed when a config file was required
  [#606][]
- Bugfix: fix description of non-configurable subcommands in config [#604][]
- Build: support pkg-config [#523][]

> ### Converting from CLI11 1.9
>
> - Removed deprecated set commands, use validators instead. [#565][]
> - The final "defaulted" bool has been removed, use `->capture_default_str()`
>   instead. Use `app.option_defaults()->always_capture_default()` to set this
>   for all future options. [#597][]
> - Use `add_option` on a complex number instead of `add_complex`, which has
>   been removed.

[#423]: https://github.com/CLIUtils/CLI11/pull/423
[#435]: https://github.com/CLIUtils/CLI11/pull/435
[#443]: https://github.com/CLIUtils/CLI11/pull/443
[#452]: https://github.com/CLIUtils/CLI11/pull/452
[#520]: https://github.com/CLIUtils/CLI11/pull/520
[#523]: https://github.com/CLIUtils/CLI11/pull/523
[#526]: https://github.com/CLIUtils/CLI11/pull/526
[#528]: https://github.com/CLIUtils/CLI11/pull/528
[#529]: https://github.com/CLIUtils/CLI11/pull/529
[#530]: https://github.com/CLIUtils/CLI11/pull/530
[#545]: https://github.com/CLIUtils/CLI11/pull/545
[#546]: https://github.com/CLIUtils/CLI11/pull/546
[#563]: https://github.com/CLIUtils/CLI11/pull/563
[#565]: https://github.com/CLIUtils/CLI11/pull/565
[#574]: https://github.com/CLIUtils/CLI11/pull/574
[#584]: https://github.com/CLIUtils/CLI11/pull/584
[#592]: https://github.com/CLIUtils/CLI11/pull/592
[#597]: https://github.com/CLIUtils/CLI11/pull/597
[#599]: https://github.com/CLIUtils/CLI11/pull/599
[#601]: https://github.com/CLIUtils/CLI11/pull/601
[#604]: https://github.com/CLIUtils/CLI11/pull/604
[#605]: https://github.com/CLIUtils/CLI11/pull/605
[#606]: https://github.com/CLIUtils/CLI11/pull/606

## Version 1.9: Config files and cleanup

Config file handling was revamped to fix common issues, and now supports reading
[TOML](https://github.com/toml-lang/toml).

Adding options is significantly more powerful with support for things like
`std::tuple` and `std::array`, including with transforms. Several new
configuration options were added to facilitate a wider variety of apps. GCC 4.7
is no longer supported.

- Config files refactored, supports TOML (may become default output in 2.0)
  [#362][]
- Added two template parameter form of `add_option`, allowing `std::optional` to
  be supported without a special import [#285][]
- `string_view` now supported in reasonable places [#300][], [#285][]
- `immediate_callback`, `final_callback`, and `parse_complete_callback` added to
  support controlling the App callback order [#292][], [#313][]
- Multiple positional arguments maintain order if `positionals_at_end` is set.
  [#306][]
- Pair/tuple/array now supported, and validators indexed to specific components
  in the objects [#307][], [#310][]
- Footer callbacks supported [#309][]
- Subcommands now support needs (including nameless subcommands) [#317][]
- More flexible type size, more useful `add_complex` [#325][], [#370][]
- Added new validators `CLI::NonNegativeNumber` and `CLI::PositiveNumber`
  [#342][]
- Transform now supports arrays [#349][]
- Option groups can be hidden [#356][]
- Add `CLI::deprecate_option` and `CLI::retire_option` functions [#358][]
- More flexible and safer Option `default_val` [#387][]
- Backend: Cleaner type traits [#286][]
- Backend: File checking updates [#341][]
- Backend: Using pre-commit to format, checked in GitHub Actions [#336][]
- Backend: Clang-tidy checked again, CMake option now `CL11_CLANG_TIDY` [#390][]
- Backend: Warning cleanup, more checks from klocwork [#350][], Effective C++
  [#354][], clang-tidy [#360][], CUDA NVCC [#365][], cross compile [#373][],
  sign conversion [#382][], and cpplint [#400][]
- Docs: CLI11 Tutorial now hosted in the same repository [#304][], [#318][],
  [#374][]
- Bugfix: Fixed undefined behavior in `checked_multiply` [#290][]
- Bugfix: `->check()` was adding the name to the wrong validator [#320][]
- Bugfix: Resetting config option works properly [#301][]
- Bugfix: Hidden flags were showing up in error printout [#333][]
- Bugfix: Enum conversion no longer broken if stream operator added [#348][]
- Build: The meson build system supported [#299][]
- Build: GCC 4.7 is no longer supported, due mostly to GoogleTest. GCC 4.8+ is
  now required. [#160][]
- Build: Restructured significant portions of CMake build system [#394][]

> ### Converting from CLI11 1.8
>
> - Some deprecated methods dropped
>   - `add_set*` should be replaced with `->check`/`->transform` and
>     `CLI::IsMember` since 1.8
>   - `get_defaultval` was replaced by `get_default_str` in 1.8
> - The true/false 4th argument to `add_option` is expected to be removed in
>   2.0, use `->capture_default_str()` since 1.8

[#160]: https://github.com/CLIUtils/CLI11/pull/160
[#285]: https://github.com/CLIUtils/CLI11/pull/285
[#286]: https://github.com/CLIUtils/CLI11/pull/286
[#290]: https://github.com/CLIUtils/CLI11/pull/290
[#292]: https://github.com/CLIUtils/CLI11/pull/292
[#299]: https://github.com/CLIUtils/CLI11/pull/299
[#300]: https://github.com/CLIUtils/CLI11/pull/300
[#301]: https://github.com/CLIUtils/CLI11/pull/301
[#304]: https://github.com/CLIUtils/CLI11/pull/304
[#306]: https://github.com/CLIUtils/CLI11/pull/306
[#307]: https://github.com/CLIUtils/CLI11/pull/307
[#309]: https://github.com/CLIUtils/CLI11/pull/309
[#310]: https://github.com/CLIUtils/CLI11/pull/310
[#313]: https://github.com/CLIUtils/CLI11/pull/313
[#317]: https://github.com/CLIUtils/CLI11/pull/317
[#318]: https://github.com/CLIUtils/CLI11/pull/318
[#320]: https://github.com/CLIUtils/CLI11/pull/320
[#325]: https://github.com/CLIUtils/CLI11/pull/325
[#333]: https://github.com/CLIUtils/CLI11/pull/333
[#336]: https://github.com/CLIUtils/CLI11/pull/336
[#341]: https://github.com/CLIUtils/CLI11/pull/341
[#342]: https://github.com/CLIUtils/CLI11/pull/342
[#348]: https://github.com/CLIUtils/CLI11/pull/348
[#349]: https://github.com/CLIUtils/CLI11/pull/349
[#350]: https://github.com/CLIUtils/CLI11/pull/350
[#354]: https://github.com/CLIUtils/CLI11/pull/354
[#356]: https://github.com/CLIUtils/CLI11/pull/356
[#358]: https://github.com/CLIUtils/CLI11/pull/358
[#360]: https://github.com/CLIUtils/CLI11/pull/360
[#362]: https://github.com/CLIUtils/CLI11/pull/362
[#365]: https://github.com/CLIUtils/CLI11/pull/365
[#370]: https://github.com/CLIUtils/CLI11/pull/370
[#373]: https://github.com/CLIUtils/CLI11/pull/373
[#374]: https://github.com/CLIUtils/CLI11/pull/374
[#382]: https://github.com/CLIUtils/CLI11/pull/382
[#387]: https://github.com/CLIUtils/CLI11/pull/387
[#390]: https://github.com/CLIUtils/CLI11/pull/390
[#394]: https://github.com/CLIUtils/CLI11/pull/394
[#400]: https://github.com/CLIUtils/CLI11/pull/400

### Version 1.9.1: Backporting fixes

This is a patch version that backports fixes from the development of 2.0.

- Support relative inclusion [#475][]
- Fix cases where spaces in paths could break CMake support [#471][]
- Fix an issue with string conversion [#421][]
- Cross-compiling improvement for Conan.io [#430][]
- Fix option group default propagation [#450][]
- Fix for C++20 [#459][]
- Support compiling with RTTI off [#461][]

[#421]: https://github.com/CLIUtils/CLI11/pull/421
[#430]: https://github.com/CLIUtils/CLI11/pull/430
[#450]: https://github.com/CLIUtils/CLI11/pull/450
[#459]: https://github.com/CLIUtils/CLI11/pull/459
[#461]: https://github.com/CLIUtils/CLI11/pull/461
[#471]: https://github.com/CLIUtils/CLI11/pull/471
[#475]: https://github.com/CLIUtils/CLI11/pull/475

## Version 1.8: Transformers, default strings, and flags

Set handling has been completely replaced by a new backend that works as a
Validator or Transformer. This provides a single interface instead of the 16
different functions in App. It also allows ordered collections to be used,
custom functions for filtering, and better help and error messages. You can also
use a collection of pairs (like `std::map`) to transform the match into an
output. Also new are inverted flags, which can cancel or reduce the count of
flags, and can also support general flag types. A new `add_option_fn` lets you
more easily program CLI11 options with the types you choose. Vector options now
support a custom separator. Apps can now be composed with unnamed subcommand
support. The final bool "defaults" flag when creating options has been replaced
by `->capture_default_str()` (ending an old limitation in construction made this
possible); the old method is still available but may be removed in future
versions.

- Replaced default help capture: `.add_option("name", value, "", True)` becomes
  `.add_option("name", value)->capture_default_str()` [#242][]
- Added `.always_capture_default()` [#242][]
- New `CLI::IsMember` validator replaces set validation [#222][]
- `IsMember` also supports container of pairs, transform allows modification of
  result [#228][]
- Added new Transformers, `CLI::AsNumberWithUnit` and `CLI::AsSizeValue`
  [#253][]
- Much more powerful flags with different values [#211][], general types
  [#235][]
- `add_option` now supports bool due to unified bool handling [#211][]
- Support for composable unnamed subcommands [#216][]
- Reparsing is better supported with `.remaining_for_passthrough()` [#265][]
- Custom vector separator using `->delimiter(char)` [#209][], [#221][], [#240][]
- Validators added for IP4 addresses and positive numbers [#210][] and numbers
  [#262][]
- Minimum required Boost for optional Optionals has been corrected to 1.61
  [#226][]
- Positionals can stop options from being parsed with `app.positionals_at_end()`
  [#223][]
- Added `validate_positionals` [#262][]
- Positional parsing is much more powerful [#251][], duplicates supported
  [#247][]
- Validators can be negated with `!` [#230][], and now handle tname functions
  [#228][]
- Better enum support and streaming helper [#233][] and [#228][]
- Cleanup for shadow warnings [#232][]
- Better alignment on multiline descriptions [#269][]
- Better support for aarch64 [#266][]
- Respect `BUILD_TESTING` only if CLI11 is the main project; otherwise,
  `CLI11_TESTING` must be used [#277][]
- Drop auto-detection of experimental optional and boost::optional; must be
  enabled explicitly (too fragile) [#277][] [#279][]

> ### Converting from CLI11 1.7
>
> - `.add_option(..., true)` should be replaced by
>   `.add_option(...)->capture_default_str()` or
>   `app.option_defaults()->always_capture_default()` can be used
> - `app.add_set("--name", value, {"choice1", "choice2"})` should become
>   `app.add_option("--name", value)->check(CLI::IsMember({"choice1", "choice2"}))`
> - The `_ignore_case` version of this can be replaced by adding
>   `CLI::ignore_case` to the argument list in `IsMember`
> - The `_ignore_underscore` version of this can be replaced by adding
>   `CLI::ignore_underscore` to the argument list in `IsMember`
> - The `_ignore_case_underscore` version of this can be replaced by adding both
>   functions listed above to the argument list in `IsMember`
> - If you want an exact match to the original choice after one of the modifier
>   functions matches, use `->transform` instead of `->check`
> - The `_mutable` versions of this can be replaced by passing a pointer or
>   shared pointer into `IsMember`
> - An error with sets now produces a `ValidationError` instead of a
>   `ConversionError`

[#209]: https://github.com/CLIUtils/CLI11/pull/209
[#210]: https://github.com/CLIUtils/CLI11/pull/210
[#211]: https://github.com/CLIUtils/CLI11/pull/211
[#216]: https://github.com/CLIUtils/CLI11/pull/216
[#221]: https://github.com/CLIUtils/CLI11/pull/221
[#222]: https://github.com/CLIUtils/CLI11/pull/222
[#223]: https://github.com/CLIUtils/CLI11/pull/223
[#226]: https://github.com/CLIUtils/CLI11/pull/226
[#228]: https://github.com/CLIUtils/CLI11/pull/228
[#230]: https://github.com/CLIUtils/CLI11/pull/230
[#232]: https://github.com/CLIUtils/CLI11/pull/232
[#233]: https://github.com/CLIUtils/CLI11/pull/233
[#235]: https://github.com/CLIUtils/CLI11/pull/235
[#240]: https://github.com/CLIUtils/CLI11/pull/240
[#242]: https://github.com/CLIUtils/CLI11/pull/242
[#247]: https://github.com/CLIUtils/CLI11/pull/247
[#251]: https://github.com/CLIUtils/CLI11/pull/251
[#253]: https://github.com/CLIUtils/CLI11/pull/253
[#262]: https://github.com/CLIUtils/CLI11/pull/262
[#265]: https://github.com/CLIUtils/CLI11/pull/265
[#266]: https://github.com/CLIUtils/CLI11/pull/266
[#269]: https://github.com/CLIUtils/CLI11/pull/269
[#277]: https://github.com/CLIUtils/CLI11/pull/277
[#279]: https://github.com/CLIUtils/CLI11/pull/279
