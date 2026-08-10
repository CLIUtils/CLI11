# Subcommands and the App {#book-subcommands}

Subcommands are keyword that invoke a new set of options and features. For
example, the `git` command has a long series of subcommands, like `add` and
`commit`. Each can have its own options and implementations. This chapter will
focus on implementations that are contained in the same C++ application, though
the system git uses to extend the main command by calling other commands in
separate executables is supported too; that's called "Prefix commands" and is
included at the end of this chapter.

## The parent App

We'll start by discussing the parent `App`. You've already used it quite a bit,
to create options and set option defaults. There are several other things you
can do with an `App`, however.

You are given a lot of control the help output. You can set a footer with
`app.footer("My Footer")`. You can replace the default help print when a
`ParseError` is thrown with `app.failure_message(CLI::FailureMessage::help)`.
The default is `CLI::FailureMessage::simple`, and you can easily define a new
one. Just make a (lambda) function that takes an App pointer and a reference to
an error code (even if you don't use them), and returns a string.

### Inspecting the app

An app can be queried after, or during, the parse:

- `app.get_option("--name")` returns the option pointer, and throws
  `CLI::OptionNotFound` if there is none. `app["--name"]` is the same thing on a
  const app, and `get_option_no_throw` returns `nullptr` instead of throwing.
- `app.get_options()` returns all options; `app.get_subcommands()` returns the
  subcommands that were parsed, in order.
- `app.count("--name")` counts one option, and `app.count_all()` counts every
  option and subcommand use in the app.
- `app.parse_order()` returns the options in the order they appeared on the
  command line, which is how you recover the relative order of two different
  options. See the
  [inter_argument_order.cpp](https://github.com/CLIUtils/CLI11/blob/main/examples/inter_argument_order.cpp)
  example.
- `app.get_parent()` returns the parent app of a subcommand, or `nullptr` for
  the main app.

Options and subcommands can also be taken back out, which is mostly useful when
you build an app from reusable pieces:

```cpp
app.remove_option(opt_pointer);
app.remove_subcommand(sub_pointer);
```

Both return `true` if the item was found and removed.

## Adding a subcommand

Subcommands can be added just like an option:

```cpp
CLI::App* sub = app.add_subcommand("sub", "This is a subcommand");
```

The subcommand should have a name as the first argument, and a little
description for the second argument. A pointer to the internally stored
subcommand is provided; you usually will be capturing that pointer and using it
later (though you can use callbacks if you prefer). As always, feel free to use
`auto sub = ...` instead of naming the type.

You can check to see if the subcommand was received on the command line several
ways:

```cpp
if(*sub) ...
if(sub->parsed()) ...
if(app.got_subcommand(sub)) ...
if(app.got_subcommand("sub")) ...
```

You can also get a list of subcommands with `get_subcommands()`, and they will
be in parsing order.

There are a lot of options that you can set on a subcommand; in fact,
subcommands have exactly the same options as your main app, since they are
actually the same class of object (as you may have guessed from the type above).
This has the pleasant side affect of making subcommands infinitely nestable.

## Required subcommands

Each App has controls to set the number of subcommands you expect. This is
controlled by:

```cpp
app.require_subcommand(/* min */ 0, /* max */ 1);
```

If you set the max to 0, CLI11 will allow an unlimited number of subcommands.
After the (non-unlimited) maximum is reached, CLI11 will stop trying to match
subcommands. So the if you pass "`one two`" to a command, and both `one` and
`two` are subcommands, it will depend on the maximum number as to whether the
"`two`" is a subcommand or an argument to the "`one`" subcommand.

As a shortcut, you can also call the `require_subcommand` method with one
argument; that will be the fixed number of subcommands if positive, it will be
the maximum number if negative. Calling it without an argument will set the
required subcommands to 1 or more.

The maximum number of subcommands is inherited by subcommands. This allows you
to set the maximum to 1 once at the beginning on the parent app if you only want
single subcommands throughout your app. You should keep this in mind, if you are
dealing with lots of nested subcommands.

## Using callbacks

You've already seen how to check to see what subcommands were given. It's often
much easier, however, to just define the code you want to run when you are
making your parser, and not run a bunch of code after `CLI11_PARSE` to analyse
the state (Procedural! Yuck!). You can do that with lambda functions. A
`std::function<void()>` callback `.callback()` is provided, and CLI11 ensures
that all options are prepared and usable by reference capture before entering
the callback. An example is shown below in the `geet` program.

### The three callbacks

`.callback()` sets the final callback. There are three call points in total, and
each one answers a different need:

| Callback                     | When it runs                                                                                                            |
| ---------------------------- | ----------------------------------------------------------------------------------------------------------------------- |
| `preparse_callback(f)`       | One time, after the first argument of the app or subcommand is seen. `f` takes the number of arguments left to process. |
| `parse_complete_callback(f)` | As soon as the subcommand finishes parsing. Runs one time per use, so it can run several times.                         |
| `final_callback(f)`          | One time, after all processing is complete. This is what `callback()` sets.                                             |

The order for a whole app is: every subcommand `parse_complete_callback`, then
the main app `parse_complete_callback`, then the used subcommand
`final_callback`s, then the option group `final_callback`s, and last the main
app `final_callback`.

Configuration files matter here. A `parse_complete_callback` on a named
subcommand sees no data from a config file, because it runs first. A
`final_callback` runs after config processing. For option groups the
`parse_complete_callback` runs after the config file is read.

A subcommand is finished, and so its `parse_complete_callback` fires, when any
of these happen:

1. There are no more arguments.
2. Another subcommand appears that does not fit an optional positional slot.
3. The positional mark `--` appears and no positional slots are left.
4. The subcommand terminator `++` appears.

Calling a subcommand a second time resets its options and can trigger the
callback again. `.immediate_callback()` is the shorthand for moving the callback
you set with `callback()` to the parse-complete point.

### Triggering subcommands from the command line

A subcommand option can be given without entering the subcommand, using dot
notation:

```text
--sub.long=val
--sub.long val
--sub.f val
--sub1.subsub.f val
```

`--sub.long <args>` is the same as `sub --long <args> ++`, where `++` closes the
subcommand again. The names may be quoted following the TOML rules, which is how
you reach a subcommand whose name contains dots:
`"subcommand.with.dots".arg1 = value`.

## Inheritance of defaults

The following values are inherited when you add a new subcommand. This happens
at the point the subcommand is created:

- The name and description for the help flag
- The footer
- The usage
- The failure message printer function
- The formatter
- The config formatter
- Option defaults
- Allow extras
- Allow config extras
- Prefix command
- Immediate callback
- Ignore case
- Ignore underscore
- Allow Windows style options
- Fallthrough
- Group name
- Max required subcommands
- prefix_matching
- Configurable
- validate positional arguments
- validate optional arguments

## More subcommand modifiers

Some further modifiers apply to an app, a subcommand, or an option group:

| Modifier                              | Description                                                                                                                                                                                                          |
| ------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `->require_option()`                  | Require 1 or more options or option groups. Also `require_option(N)` and `require_option(min, max)`, like `require_subcommand`.                                                                                      |
| `->needs(opt_or_sub)`                 | The given option or subcommand must be used before this subcommand passes validation.                                                                                                                                |
| `->excludes(opt_or_sub)`              | The given option or subcommand cannot be used together with this one.                                                                                                                                                |
| `->disabled()`                        | Turn the subcommand off. Takes an optional bool.                                                                                                                                                                     |
| `->disabled_by_default()`             | Disable at the start of each parse, so another subcommand can turn it on.                                                                                                                                            |
| `->enabled_by_default()`              | Enable at the start of each parse, so another subcommand can turn it off.                                                                                                                                            |
| `->positionals_at_end()`              | Positional arguments must come after all options.                                                                                                                                                                    |
| `->configurable()`                    | Allow the subcommand to be triggered from a configuration file. By default config file entries only update defaults.                                                                                                 |
| `->alias(name)`                       | Add another name for the subcommand.                                                                                                                                                                                 |
| `->group(name)`                       | Set the help group for the subcommand. An empty name hides it from the help.                                                                                                                                         |
| `->allow_non_standard_option_names()` | Accept long option names with a single dash, such as `-single`. Not recommended, but useful when you reproduce an existing interface. A short option may not share its first character with a single dash long name. |

`disabled_by_default` and `enabled_by_default` are the building blocks behind
`CLI::TriggerOn` and `CLI::TriggerOff`, described in @ref book-option-groups.

## Help and version flags

Every app is created with a help flag. You can rename it, replace it, or remove
it:

```cpp
app.set_help_flag("-h,--help", "Print this help message and exit");
app.set_help_flag();  // pass nothing to remove it
```

`set_help_all_flag` adds a second flag that expands every subcommand in the help
output:

```cpp
app.set_help_all_flag("--help-all", "Expand all help");
```

A version flag is not created for you. Add one with a fixed string, or with a
callback that produces the string when the flag is used:

```cpp
app.set_version_flag("--version", std::string(CLI11_VERSION));
app.set_version_flag("--version", []() { return my_version_string(); });
```

All three functions return the option pointer, so the usual option modifiers
apply, and all three replace the existing flag if there is one. The pointers can
be read back later with `get_help_ptr()`, `get_help_all_ptr()`, and
`get_version_ptr()`.

## Special modes

There are several special modes for Apps and Subcommands.

### Allow extras

Normally CLI11 throws an error if you don't match all items given on the command
line. However, you can enable `allow_extras()` to instead store the extra values
in `.remaining()`. You can get all remaining options including those in
contained subcommands recursively in the original order with `.remaining(true)`.
`.remaining_size()` is also provided; this counts the size but ignores the `--`
special separator if present.

### Fallthrough

Fallthrough allows an option that does not match in a subcommand to "fall
through" to the parent command; if that parent allows that option, it matches
there instead. This was added to allow CLI11 to represent models:

```text
./my_program my_model_1 --model_flag --shared_flag
```

Here, `--shared_flag` was set on the main app, and on the command line it "falls
through" `my_model_1` to match on the main app. This is set through
`->fallthrough()` on a subcommand.

calling help on subcommands with fallthrough will result in the parent options
showing as if they were part of the subcommand.

#### Subcommand fallthrough

Subcommand fallthrough allows additional subcommands to be triggered after the
first subcommand. By default subcommand fallthrough is enabled, but it can be
turned off through `->subcommand_fallthrough(false)` on a subcommand. This will
prevent additional subcommands at the same inheritance level from triggering,
the strings would then be treated as positional values. As a technical note if
fallthrough is enabled but subcommand fallthrough disabled (this is not the
default in both cases), then subcommands on grandparents can still be triggered
from the grandchild subcommand, unless subcommand fallthrough is also disabled
on the parent. This is an unusual circumstance but may arise in some very
particular situations.

### Prefix command

This is a special mode that allows "prefix" commands, where the parsing
completely stops when it gets to an unknown option. Further unknown options are
ignored, even if they could match. Git is the traditional example for prefix
commands; if you run git with an unknown subcommand, like "`git thing`", it then
calls another command called "`git-thing`" with the remaining options intact.

### prefix matching

A modifier is available for subcommand matching,
`->allow_subcommand_prefix_matching()`. if this is enabled unambiguous prefix
portions of a subcommand will match. For Example `upgrade_package` would match
on `upgrade_`, `upg`, `u` as long as no other subcommand would also match. It
also disallows subcommand names that are full prefixes of another subcommand.

### Silent subcommands

Subcommands can be modified by using the `silent` option. This will prevent the
subcommand from showing up in the get_subcommands list. This can be used to make
subcommands into modifiers. For example, a help subcommand might look like

```c++
    auto sub1 = app.add_subcommand("help")->silent();
    sub1->parse_complete_callback([]() { throw CLI::CallForHelp(); });
```

This would allow calling help such as:

```bash
./app help
./app help sub1
```

### Positional Validation

Some arguments supplied on the command line may be legitimately applied to more
than 1 positional argument. In this context calling `validate_positionals()` on
the application or subcommand will check any validators before applying the
command line argument to the positional option. It is not an error to fail
validation in this context, positional arguments not matching any validators
will be treated as extra arguments (retrievable via `app.remaining()`) which may
generate an error depending on settings.

### Optional Argument Validation

Similar to positional validation, there are occasional contexts in which case it
might be ambiguous whether an argument should be applied to an option or a
positional option.

```c++
    std::vector<std::string> vec;
    std::vector<int> ivec;
    app.add_option("pos", vec);
    app.add_option("--args", ivec)->check(CLI::Number);
    app.validate_optional_arguments();
```

In this case a sequence of integers is expected for the argument and remaining
strings go to the positional string vector. Without the
`validate_optional_arguments()` active it would be impossible get any later
arguments into the positional if the `--args` option is used. The validator in
this context is used to make sure the optional arguments match with what the
argument is expecting and if not the `-args` option is closed, and remaining
arguments fall into the positional.
