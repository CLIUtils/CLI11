[![Build Status](https://travis-ci.org/henryiii/CLI11.svg?branch=master)](https://travis-ci.org/henryiii/CLI11)

# CLI11

Why write another CLI parser?


The following attributes were deemed most important in a CLI parser library:

* Easy to include (i.e., header only, one file if possible, no external requirements): While many programs depend on Boost, that should not be a requirement if all you want is CLI parsing.
* Short Syntax: This is one of the main points of a CLI parser, it should make variables from the command line nearly as easy to define as any other variables. If most of your program is hidden in CLI parsing, this is a problem for readability.
* Work with GCC 4.8 (CentOS 7) or above, or MacOS Clang (C++11).
* Well tested using Travis.
* Good help printing (in progress).
* Standard idioms supported naturally, like flags.
* Easy to execute, with help, parse errors, etc. providing correct exit and details.
* Easy to extend as part of a framework that provides "applications".
* Simple support for subcommands,

The major CLI parsers out there include:

* [Boost Program Options](http://www.boost.org/doc/libs/1_63_0/doc/html/program_options.html): A great library if you already depend on Boost, it's pre-C++11 syntax is really odd and setting up the correct call in the main function is poorly documented. A simple wrapper for the Boost library (Program.hpp) is provided in this package if you already link to Boost.
* [The Lean Mean C++ Option Parser](http://optionparser.sourceforge.net): One header file is great, but the syntax is atrocious, IMO.
* [TCLAP](http://tclap.sourceforge.net): Not quite standard command line parsing, seems to not be well supported anymore. Header only, but in quite a few files. Not even moved to GitHub yet.
* [Cxxopts](https://github.com/jarro2783/cxxopts): C++ 11, single file, and nice CMake support, but requires regex, therefore GCC 4.8 (CentOS 7 default) does not work. Syntax closely based on Boost PO.

So, this library is an attempt to provide a great syntax, good compatibility, and minimal installation.

## Installing

To use, copy CLI.hpp into your include directory, and you are set.

To build the tests, get the entire directory and use CMake:

```bash
mkdir build
cd build
cmake ..
make
GTEST_COLOR=1 CTEST_OUTPUT_ON_FAILURE=1 make test
```

## Syntax 1

There are currently two supported syntaxes. I have not decided which is the most useful yet, and would be open to comments. The first syntax is:

```cpp
CLI::App app{"App description"};

std::string filename = "default";
app.add_option("-f,--file", file, "A help string");

try {
    app.run(argc, argv);
} catch (const CLI::Error &e) {
    return app.exit(e);
}
```

The initialization is just one line, adding options is just two each. The try/catch block ensures that `-h,--help` or a parse error will exit with the correct return code. (The return here should be inside `main`). After the app runs, the filename will be set to the correct value if it was passed, otherwise it will be set to the default. You can check to see if this was passed on the command line with `app.count("--file")`.

The supported values are:

```
app.add_options(option_name,
                variable_to_bind_to, // int, float, vector, or string-like
                help_string,
                flags, ...)          // Listed below

app.add_flag(option_name,
             optional_intlike_to_bind_to,
             help_string)

app.add_set(option_name,
            variable_to_bind_to,
            set_of_possible_options,
            flags, ...)

App* subcom = app.add_subcommand(name, discription);

```


There are several flags:

* `CLI::Default`: Print the default value in help
* `CLI::Required`: The program will quit if this option is not present
* `CLI::Opts(N)`: Take `N` values instead of as many as possible, only for vector args
* `CLI::ExistingFile`: Requires that the file exists if given
* `CLI::ExistingDirectory`: Requires that the directory exists
* `CLI::NonexistentPath`: Requires that the path does not exist

Options can be given as:

* `-a` (flag)
* `-abc` (flags can be combined)
* `-f filename` (option)
* `-ffilename` (no space required)
* `-abcf filename` (flags and option can be combined)
* `--long` (long flag)
* `--file filename` (space)
* `--file=filename` (equals)

An option must start with a alphabetic character or underscore. For long options, anything but an equals sign or a comma is valid after that. Names are given as a comma separated string, with the dash or dashes. An option or flag can have as many names as you want, and afterward, using `count`, you can use any of the names, with dashes as needed, to count the options. One of the names is allowed to be given without proceeding dash(es); if present the option is a positional option, and that name will be used on help line for its positional form.

Extra positional arguments will cause the program to exit, so at least one positional option with a vector is recommended if you want to allow extraneous arguments
If `--` is present in the command line,
everything after that is positional only.


## Subcommands

Subcommands are naturally supported, with an infinite depth. To add a subcommand, call the `add_subcommand` method with a name and an optional description. This gives a pointer to an `App` that behaves just like the main app, and can take options or further subcommands.

All `App`s have a `get_subcommand()` method, which returns a pointer to the subcommand passed on the command line, or `nullptr` if no subcommand was given. A simple compare of this pointer to each subcommand allows choosing based on subcommand. For many cases, however, using an app's callback may be easier. Every app executes a callback function after it parses; just use a lambda function (with capture to get parsed values) to `.add_callback`. If you throw CLI::Success, you can
even exit the program through the callback. The main `App` has a callback slot, as well, but it is generally not as useful.



> ## Subclassing
> 
> The App class was designed allow toolkits to subclass it, to provide default options and setup/teardown code. Subcommands remain `App`'s, since those are not expected to need setup and teardown. The default `App` only adds a help flag, `-h,--help`.
>
> Also, in a related note, the `App`s you get a pointer to are stored in the parent `App` and cannot be deleted.

## Make syntax
 
A second, provisional syntax looks like this:

```cpp
CLI::App app{"App description"};

auto filename = app.add_option("-f,--file", "default", "A help string");
auto int_value = app.add_option<int>("-i,--int", "An int with no default");

try {
    app.run(argc, argv);
} catch (const CLI::Error &e) {
    return app.exit(e);
}

std::cout << "The file was: " << *filename << std::endl;
std::cout << "This will throw an error if int not passed: " << *int_value << std::endl;
```


Internally, it uses the same mechanism to work, it just provides a single line definition, but requires a template argument for non-strings, and creates an object that must be dereferenced to be used. This object (`CLI::Value<type>`) supports conversion to bool, allowing you to easily check if an option was passed without resorting to count. Dereferencing will also throw an error if no value was passed and no default was given.

The same functions as the first syntax are supported, only with `make` instead of `add`, and with the variable to bind to replaced by the default value (optional). If you want to use something other than a string option and do not want to give a default, you need to give a template parameter with the type.

Value wraps a `shared_ptr` to a `unique_ptr` to a value, so it lasts even if the `App` object is destructed.

## How it works

Every `make_` or `add_` option you've seen depends on one method that takes a lambda function. Each of these methods is just making a different lambda function with capture to populate the option. The function has full access to the vector of vector of strings, so it knows how many times an option was passed, and how many arguments each passing received (flags add empty strings to keep the counts correct). The lambda returns true if it could validate the option strings, and
false if it failed.


