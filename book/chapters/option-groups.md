# Option groups {#book-option-groups}

The `->group("name")` modifier on an option only changes where the option is
printed in the help. An option _group_ is a stronger tool: it is a real
container that collects options and can carry requirements of its own.

```cpp
auto *format = app.add_option_group("output_format", "formatting type for output");
```

`add_option_group` returns a pointer to the group. The description is optional.
An option group is a specialization of `App`, so everything that works on an app
or a subcommand works on a group as well: `require_option`, `excludes`, `needs`,
`disabled`, callbacks, and nesting.

## Adding options to a group

Options can be created directly on the group, exactly as on an app:

```cpp
std::string filename;
format->add_option("--file", filename, "output file");
format->add_flag("--binary", "write binary output");
```

An option that already exists on the parent app can be moved into the group:

```cpp
auto *opt = app.add_option("--file", filename);
format->add_option(opt);
format->add_options(opt1, opt2, opt3);
```

The option pointers must belong to the parent application of the group, or an
error is generated. A subcommand can be moved in the same way, which removes it
from its parent:

```cpp
format->add_subcommand(subcom_pointer);
```

## Requirements between groups

An app counts a whole option group as a single option for the purpose of
`require_option`, and the group counts as used if any option or subcommand
inside it was used. That is what makes groups useful: you can require one of
three options in one group and one of three in another.

```cpp
CLI::App app("data output specification");

auto *format = app.add_option_group("output_format", "formatting type for output");
format->add_flag("--csv", "write csv output");
format->add_flag("--binary", "write binary output");
format->require_option(1);  // exactly one of the two

auto *target = app.add_option_group("output_target", "target for the output");
target->add_option("--file", filename, "output file");
target->add_flag("--stdout", "write to stdout");
target->require_option(1);
```

`require_option(N)` requires exactly `N` if `N` is positive, up to `N` if `N` is
negative, and resets to "0 or more" for `N=0`. `require_option(min, max)` sets
both ends, with a `max` of 0 meaning unlimited. `require_option()` with no
argument requires 1 or more.

Disabling a group with `->disabled()` turns off every option inside it. Groups
can also contain other groups.

See the
[option_groups.cpp](https://github.com/CLIUtils/CLI11/blob/main/examples/option_groups.cpp)
and
[ranges.cpp](https://github.com/CLIUtils/CLI11/blob/main/examples/ranges.cpp)
examples.

## Triggering one group from another

`CLI::TriggerOn` and `CLI::TriggerOff` connect groups, so that use of one group
enables or disables another:

```cpp
CLI::TriggerOn(group1_pointer, triggered_group);
CLI::TriggerOff(group2_pointer, disabled_group);
```

The second argument can also be a `std::vector<App *>`. These helpers are built
from `preparse_callback`, `enabled_by_default()`, and `disabled_by_default()`;
see [Subcommands](@ref book-subcommands). Use them one time per group, since
they replace any earlier use of those three functions. For anything more
complex, write the `preparse_callback` yourself.

## Group names and the help output

The group name controls how the help print treats the group:

- A normal name prints the options under that heading.
- An empty name hides the whole group, which is a quick way to hide a set of
  options: `app.add_option_group("")`.
- A name that starts with `+` prints the options as if they were not in a group
  at all, and `get_options` treats them the same way:
  `app.add_option_group("+sub")`.

Group names may not contain newlines or null characters.

## Nameless subcommands

A subcommand with an empty name behaves much like an option group:

```cpp
auto *shared = app.add_subcommand();
shared->add_option("--shared", value);
```

If an option is not found in the main app, all nameless subcommands are searched
as well. Because `add_subcommand` also accepts a `std::shared_ptr<App>`, a set
of options can be defined in one component and merged into one or more apps.
Multiple nameless subcommands are allowed, and their callbacks run only if some
option from the group was parsed.

## Deprecating and retiring options

Two helpers manage the life cycle of an option that is on its way out:

```cpp
CLI::deprecate_option(option_pointer, "replacement_name");
CLI::deprecate_option(app, "--option_name", "replacement_name");
```

A deprecated option keeps working. The help print marks it, and the first use
prints a warning that names the replacement, if you gave one.

```cpp
CLI::retire_option(app, option_pointer);
CLI::retire_option(app, "--option_name");
```

A retired option does nothing. If the option exists it is replaced with a dummy
option that takes the same arguments, so old command lines still parse, and the
first use prints a warning. See the
[retired.cpp](https://github.com/CLIUtils/CLI11/blob/main/examples/retired.cpp)
example.
