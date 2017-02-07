[![Build Status](https://travis-ci.org/henryiii/CLI11.svg?branch=master)](https://travis-ci.org/henryiii/CLI11)

# CLI11

Why write another CLI parser?


The following attributes are what I believe are important in a CLI parser library:

* Easy to include (i.e., header only, one file if possible, no external requirements): While many programs depend on Boost, that should not be a requirement if all you want is CLI parsing.
* Short Syntax: This is one of the main points of a CLI parser, it should make variables from the command line nearly as easy to define as any other variables. If most of your program is hidden in CLI parsing, this is a problem for readability.
* Work with GCC 4.7+ (such as GCC 4.8 on CentOS 7) or above, or Clang 3.4+. Should work on Linux and MacOS.
* Well tested using Travis.
* Good help printing (in progress).
* Standard idioms supported naturally, like grouping flags, the positional seperator, etc.
* Easy to execute, with help, parse errors, etc. providing correct exit and details.
* Easy to extend as part of a framework that provides "applications".
* Human readable support for subcommands.

The major CLI parsers out there include:

* [Boost Program Options](http://www.boost.org/doc/libs/1_63_0/doc/html/program_options.html): A great library if you already depend on Boost, it's pre-C++11 syntax is really odd and setting up the correct call in the main function is poorly documented. A simple wrapper for the Boost library (Program.hpp) is provided in this package if you already link to Boost.
* [The Lean Mean C++ Option Parser](http://optionparser.sourceforge.net): One header file is great, but the syntax is atrocious, IMO.
* [TCLAP](http://tclap.sourceforge.net): Not quite standard command line parsing, seems to not be well supported anymore. Header only, but in quite a few files. Not even moved to GitHub yet.
* [Cxxopts](https://github.com/jarro2783/cxxopts): C++ 11, single file, and nice CMake support, but requires regex, therefore GCC 4.8 (CentOS 7 default) does not work. Syntax closely based on Boost PO.

So, this library was designed to provide a great syntax, good compiler compatibility, and minimal installation fuss.

## Installing

To use, there are two methods.
1. Copy `CLI11.hpp` from the [most recent release](https://github.com/henryiii/CLI11/releases) into your include directory, and you are set. This is combined from the source files  for every release.
2. Checkout the repository and add as a subdirectory for CMake. You can use the CLI interface target. (CMake 3.4+ recommended)

To build the tests, get the entire directory and use CMake:

```bash
mkdir build
cd build
cmake ..
make
GTEST_COLOR=1 CTEST_OUTPUT_ON_FAILURE=1 make test
```

## Adding options

To set up, add options, and run, your main function will look something like this:

```cpp
CLI::App app{"App description"};

std::string filename = "default";
app.add_option("-f,--file", file, "A help string");

try {
    app.run(argc, argv);
} catch (const CLI::ParseError &e) {
    return app.exit(e);
}
```

The initialization is just one line, adding options is just two each. The try/catch block ensures that `-h,--help` or a parse error will exit with the correct return code. (The return here should be inside `main`). After the app runs, the filename will be set to the correct value if it was passed, otherwise it will be set to the default. You can check to see if this was passed on the command line with `app.count("--file")`.

The supported values are:

```
app.add_options(option_name,
                variable_to_bind_to, // int, float, vector, or string-like
                help_string="",
                default=false)

app.add_flag(option_name,
             int_or_bool = nothing,
             help_string="")

app.add_set(option_name,
            variable_to_bind_to,
            set_of_possible_options,
            help_string="",
            default=false)

App* subcom = app.add_subcommand(name, discription);

```

An option name must start with a alphabetic character or underscore. For long options, anything but an equals sign or a comma is valid after that. Names are given as a comma separated string, with the dash or dashes. An option or flag can have as many names as you want, and afterward, using `count`, you can use any of the names, with dashes as needed, to count the options. One of the names is allowed to be given without proceeding dash(es); if present the option is a positional option, and that name will be used on help line for its positional form.

> ### Example
>
> * `"one,-o,--one"`: Valid as long as not a flag, would create an option that can be specified positionally, or with `-o` or `--option`
> * `"this"` Can only be passed positionally
> * `"-a,-b,-c"` No limit to the number of non-positional option names


The add commands return a pointer to an internally stored `Option`. If you set the final argument to true, the default value is captured and printed on the command line with the help flag. This option can be used direcly to check for the count (`->count()`) after parsing to avoid a string based lookup. Before parsing, you can set the following options:

* `->required()`: The program will quit if this option is not present
* `->expected(N)`: Take `N` values instead of as many as possible, only for vector args
* `->check(CLI::ExistingFile)`: Requires that the file exists if given
* `->check(CLI::ExistingDirectory)`: Requires that the directory exists
* `->check(CLI::NonexistentPath)`: Requires that the path does not exist

These options return the `Option` pointer, so you can chain them together, and even skip storing the pointer entirely. Check takes any function that has the signature `bool(std::string)`.


On the command line, options can be given as:

* `-a` (flag)
* `-abc` (flags can be combined)
* `-f filename` (option)
* `-ffilename` (no space required)
* `-abcf filename` (flags and option can be combined)
* `--long` (long flag)
* `--file filename` (space)
* `--file=filename` (equals)

Extra positional arguments will cause the program to exit, so at least one positional option with a vector is recommended if you want to allow extraneous arguments
If `--` is present in the command line,
everything after that is positional only.


## Subcommands

Subcommands are naturally supported, with an infinite depth. To add a subcommand, call the `add_subcommand` method with a name and an optional description. This gives a pointer to an `App` that behaves just like the main app, and can take options or further subcommands.

All `App`s have a `get_subcommand()` method, which returns a pointer to the subcommand passed on the command line, or `nullptr` if no subcommand was given. A simple compare of this pointer to each subcommand allows choosing based on subcommand. For many cases, however, using an app's callback may be easier. Every app executes a callback function after it parses; just use a lambda function (with capture to get parsed values) to `.add_callback`. If you throw `CLI::Success`, you can
even exit the program through the callback. The main `App` has a callback slot, as well, but it is generally not as useful.



> ## Subclassing
> 
> The App class was designed allow toolkits to subclass it, to provide default options and setup/teardown code. Subcommands remain `App`'s, since those are not expected to need setup and teardown. The default `App` only adds a help flag, `-h,--help` through the `virtual void setup()` method. If you only want to change this flag, override this method. Since subcommands are always the built in `App` object, they must have a normal help flag.
>
> Also, in a related note, the `App`s you get a pointer to are stored in the parent `App` in `unique_ptr`s (like `Option`s) and are deleted when the main `App` goes out of scope.


## How it works

Every `add_` option you've seen depends on one method that takes a lambda function. Each of these methods is just making a different lambda function with capture to populate the option. The function has full access to the vector of vector of strings, so it knows how many times an option was passed, and how many arguments each passing received (flags add empty strings to keep the counts correct). The lambda returns true if it could validate the option strings, and
false if it failed.


