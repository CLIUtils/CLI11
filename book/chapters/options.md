# Options

## Simple options
The most versatile addition to a command line program is a option. This is like a flag, but it takes an argument. CLI11 handles all the details for many types of options for you, based on their type. To add an option:


```cpp
int int_option{0};
app.add_option("-i", int_option, "Optional description");
```

This will bind the option `-i` to the integer `int_option`. On the command line, a single value that can be converted to an integer will be expected. Non-integer results will fail. If that option is not given, CLI11 will not touch the initial value. This allows you to set up defaults by simply setting your value beforehand. If you want CLI11 to display your default value, you can add the optional final argument `true` when you add the option. If you do not add this, you do not even need your option value to be printable[^1].

```cpp
int int_option{0};
app.add_option("-i", int_option, "Optional description", true);
```

You can use any C++ int-like type, not just `int`. CLI11 understands the following categories of types:

| Type        | CLI11 |
|-------------|-------|
| int-like    | Integer conversion up to 64-bit, can be unsigned |
| float-like  | Floating point conversions |
| string-like | Anything else that can be shifted into a StringStream |
| vector-like | A vector of the above three types (see below) |
| function | A function that takes an array of strings and returns a string that describes the conversion failure or empty for success. May be the empty function. (`{}`) |

By default, CLI11 will assume that an option is optional, and one value is expected if you do not use a vector. You can change this on a specific option using option modifiers.

## Positional options and aliases

When you give an option on the command line without a name, that is a positional option. Positional options are accepted in the same order they are defined. So, for example:

```term
gitbook:examples $ ./a.out one --two three four
```

The string `one` would have to be the first positional option. If `--two` is a flag, then the remaining two strings are positional. If `--two` is a one-argument option, then `four` is the second positional. If `--two` accepts two or more arguments, then there are no more positionals.

To make a positional option, you simply give CLI11 one name that does not start with a dash. You can have as many (non-overlapping) names as you want for an option, but only one positional name. So the following name string is valid:

```cpp
"-a,-b,--alpha,--beta,mypos"
```

This would make two short option aliases, two long option alias, and the option would be also be accepted as a positional.

## Vectors of options

If you use a vector instead of a plain option, you can accept more than one value on the command line. By default, a vector accepts as many options as possible, until the next value that could be a valid option name. You can specify a set number using an option modifier `->expected(N)`. (The default unlimited behavior on vectors is restore with `N=-1`) CLI11 does not differentiate between these two methods for unlimited acceptance options:[^2]

| Separate names    | Combined names  |
|-------------------|-----------------|
| `--vec 1 --vec 2` | `--vec 1 2`     |

The original version did allow the option system to access information on the grouping of options received, but was removed for simplicity.

An example of setting up a vector option:

```cpp
std::vector<int> int_vec;
app.add_option("--vec", int_vec, "My vector option");
```

Vectors will be replaced by the parsed content if the option is given on the command line.


## Option modifiers

When you call `add_option`, you get a pointer to the added option. You can use that to add option modifiers. A full listing of the option modifiers:

| Modifier | Description |
|----------|-------------|
| `->required()` | The program will quit if this option is not present. This is `mandatory` in Plumbum, but required options seems to be a more standard term. For compatibility, `->mandatory()` also works. |
| `->expected(N)` | Take `N` values instead of as many as possible, mainly for vector args. |
| `->expected(Nmin,Nmax)` | Take between `Nmin` and `Nmax` values. |
| `->needs(opt)` | This option requires another option to also be present, opt is an `Option` pointer. |
| `->excludes(opt)` | This option cannot be given with `opt` present, opt is an `Option` pointer. |
| `->envname(name)` | Gets the value from the environment if present and not passed on the command line. |
| `->group(name)` | The help group to put the option in. No effect for positional options. Defaults to `"Options"`. `"Hidden"` will not show up in the help print. |
| `->ignore_case()` | Ignore the case on the command line (also works on subcommands, does not affect arguments). |
| `->ignore_underscore()` | Ignore any underscores on the command line (also works on subcommands, does not affect arguments, new in CLI11 1.7). |
| `->allow_extra_args()` | Allow extra argument values to be included when an option is passed. Enabled by default for vector options. |
| `->multi_option_policy(CLI::MultiOptionPolicy::Throw)` | Sets the policy for handling multiple arguments if the option was received on the command line several times. `Throw`ing an error is the default, but `TakeLast`, `TakeFirst`, `TakeAll`, and `Join` are also available. See the next three lines for shortcuts to set this more easily. |
| `->take_last()` | Only use the last option if passed several times. This is always true by default for bool options, regardless of the app default, but can be set to false explicitly with `->multi_option_policy()`. |
| `->take_first()` | sets `->multi_option_policy(CLI::MultiOptionPolicy::TakeFirst)` |
| `->join()` | sets `->multi_option_policy(CLI::MultiOptionPolicy::Join)`, which uses newlines or the specified delimiter to join all arguments into a single string output. |
| `->join(delim)` | sets `->multi_option_policy(CLI::MultiOptionPolicy::Join)`, which uses `delim` to join all arguments into a single string output. |
| `->check(CLI::ExistingFile)` | Requires that the file exists if given. |
| `->check(CLI::ExistingDirectory)` | Requires that the directory exists. |
| `->check(CLI::NonexistentPath)` | Requires that the path does not exist. |
| `->check(CLI::Range(min,max))` | Requires that the option be between min and max (make sure to use floating point if needed). Min defaults to 0. |
| `->each(void(std::string))` | Run a function on each parsed value, *in order*. |

The `->check(...)` modifiers adds a callback function of the form `bool function(std::string)` that runs on every value that the option receives, and returns a value that tells CLI11 whether the check passed or failed.

## Using the `CLI::Option` pointer

Each of the option creation mechanisms returns a pointer to the internally stored option. If you save that pointer, you can continue to access the option, and change setting on it later. The Option object can also be converted to a bool to see if it was passed, or `->count()` can be used to see how many times the option was passed. Since flags are also options, the same methods work on them.

```cpp
CLI::Option* opt = app.add_flag("--opt");

CLI11_PARSE(app, argv, argc);

if(* opt)
    std::cout << "Flag received " << opt->count() << " times." << std::endl;
```

## Inheritance of defaults

One of CLI11's systems to allow customizability without high levels of verbosity is the inheritance system. You can set default values on the parent `App`, and all options and subcommands created from it remember the default values at the point of creation. The default value for Options, specifically, are accessible through the `option_defaults()` method. There are four settings that can be set and inherited:

*   `group`: The group name starts as "Options"
*   `required`: If the option must be given. Defaults to `false`. Is ignored for flags.
*   `multi_option_policy`: What to do if several copies of an option are passed and one value is expected. Defaults to `CLI::MultiOptionPolicy::Throw`. This is also used for bool flags, but they always are created with the value `CLI::MultiOptionPolicy::TakeLast` regardless of the default, so that multiple bool flags does not cause an error. But you can override that flag by flag.
*   `ignore_case`: Allow any mixture of cases for the option or flag name

An example of usage:

```
app.option_defaults()->ignore_case()->group("Required");

app.add_flag("--CaSeLeSs");
app.get_group() // is "Required"
```

Groups are mostly for visual organization, but an empty string for a group name will hide the option.


## Listing of specialty options:

Besides `add_option` and `add_flag`, there are several special ways to create options for sets and complex numbers.

### Sets

You can add a set with `add_set`, where you give a variable to set and a `std::set` of choices to pick from. There also is a `add_set_ignore_case` version which ignores case when set matching. If you use an existing set instead of an inline one, you can edit the set after adding it and changes will be reflected in the set checking and help message.

```cpp
int val{0};
app.add_set("--even", val, {0,2,4,6,8});
```

### Complex numbers

You can also add a complex number. This type just needs to support a `(T x, T y)` constructor and be printable. You can also pass one extra argument that will set the label of the type; by default it is "COMPLEX".

```cpp
std::complex<float> val{0.0F,0.0F};
app.add_complex("--cplx", val);
```

### Windows style options (New in CLI11 1.7)

You can also set the app setting `app->allow_windows_style_options()` to allow windows style options to also be recognized on the command line:

*   `/a` (flag)
*   `/f filename` (option)
*   `/long` (long flag)
*   `/file filename` (space)
*   `/file:filename` (colon)

Windows style options do not allow combining short options or values not separated from the short option like with `-` options. You still specify option names in the same manor as on Linux with single and double dashes when you use the `add_*` functions, and the Linux style on the command line will still work. If a long and a short option share the same name, the option will match on the first one defined.

## Parse configuration

How an option and its arguments are parsed depends on a set of controls that are part of the option structure.  In most circumstances these controls are set automatically based on the function used to create the option and the type the arguments are parsed into.  The variables define the size of the underlying type (essentially how many strings make up the type), the expected size (how many groups are expected) and a flag indicating if multiple groups are allowed with a single option.  And these interact with the `multi_option_policy` when it comes time to parse.

### examples
How options manage this is best illustrated through some examples
```cpp
std::string val;
app.add_option("--opt",val,"description");
```
creates an option that assigns a value to a `std::string`  When this option is constructed it sets a type_size of 1.  meaning that the assignment uses a single string.  The Expected size is also set to 1 by default, and `allow_extra_args` is set to false. meaning that each time this option is called 1 argument is expected.  This would also be the case if val were a `double`, `int` or any other single argument types.

now for example
```cpp
std::pair<int, std::string> val;
app.add_option("--opt",val,"description");
```

In this case the typesize is automatically detected to be 2 instead of 1, so the parsing would expect 2 arguments associated with the option.

```cpp
std::vector<int> val;
app.add_option("--opt",val,"description");
```

detects a type size of 1, since the underlying element type is a single string, so the minimum number of strings is 1.  But since it is a vector the expected number can be very big.  The default for a vector is (1<<30), and the allow_extra_args is set to true.  This means that at least 1 argument is expected to follow the option, but arbitrary numbers of arguments may follow.  These are checked if they have the form of an option but if not they are added to the argument.

```cpp
std::vector<std::tuple<int, double, std::string>> val;
app.add_option("--opt",val,"description");
```
gets into the complicated cases where the type size is now 3.  and the expected max is set to a large number and `allow_extra_args` is set to true.  In this case at least 3 arguments are required to follow the option,  and subsequent groups must come in groups of three, otherwise an error will result.

```cpp
bool val{false};
app.add_flag("--opt",val,"description");
```

Using the add_flag methods for creating options creates an option with an expected size of 0, implying no arguments can be passed.

### Customization

The `type_size(N)`, `type_size(Nmin, Nmax)`, `expected(N)`, `expected(Nmin,Nmax)`, and `allow_extra_args()` can be used to customize an option.  For example

```cpp
std::string val;
auto opt=app.add_flag("--opt{vvv}",val,"description");
opt->expected(0,1);
```
will create a hybrid option, that can exist on its own in which case the value "vvv" is used or if a value is given that value will be used.

[^1]: For example, enums are not printable to `std::cout`.
[^2]: There is a small difference. An combined unlimited option will not prioritize over a positional that could still accept values.
