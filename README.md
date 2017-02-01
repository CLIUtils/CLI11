[![Build Status](https://travis-ci.org/henryiii/CLI11.svg?branch=master)](https://travis-ci.org/henryiii/CLI11)

# CLI11

Why write another CLI parser?


The following attributes were deemed most important in a CLI parser library:

* Easy to include (i.e., header only, one file if possible): While many programs depend on Boost, that should not be a requirement if all you want is CLI parsing.
* Short Syntax: This is one of the main points of a CLI parser, it should make variables from the command line nearly as easy to define as any other variables. If most of your program is hidden in CLI parsing, this is a problem for readability.
* Work with GCC 4.8 (CentOS 7) or above, or MacOS Clang (C++11)
* Well tested
* Good help printing (in progress)
* Standard idioms supported naturally, like flags
* Easy to execute, with help, parse errors, etc. providing correct exit and details.

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

The initialization is just one line, adding options is just two each. The try/catch block ensures that `-h,--help` or a parse error will exit with the correct return code. (The return here should be inside `main`). After the app runs, the filename will be set to the correct value if it was passed, otherwise it will be set to the default. You can check to see if this was passed on the command line with `app.count("file")`.

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

```

There are several flags:

* `CLI::DEFAULT`: Print the default value in help
* `CLI::POSITIONAL`: Accept this option also as positional (or only as positional, if nameless)
* `CLI::REQUIRED`: The program will quit if this option is not present
* `CLI::OPTS(N)`: Take `N` values instead of as many as possible, only for vector args

Options can be given as:

* `-a` (flag)
* `-abc` (flags can be combined)
* `-f filename` (option)
* `-ffilename` (no space required)
* `-abcf filename` (flags and option can be combined)
* `--long` (long flag)
* `--file filename` (space)
* `--file=filename` (equals)

An option must start with a alphabetic character or underscore. For long options, anything but an equals sign is valid after that. Names are given as a comma separated string, with optional dash or dashes (the only way to get a one char long name is to be explicit with the dashes, however). An option or flag can have as many as you want, and afterward, using `count`, you can use any of the names, with optional dashes, to count the options.

Extra positional arguments will cause the program to exit, so at least one `CLI::POSITIONAL` option with a vector is recommended if you want to allow extraneous arguments
If `--` is present in the command line,
everything after that is positional only.

## Syntax 2

The second syntax looks like this:

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


Internally, it uses the same mechanism to work, it just provides a single line definition, but requires a template argument for non-strings, and creates an object that must be dereferenced to be used. This object (CLI::Value<type>) supports conversion to bool, allowing you to easily check if an option was passed without resorting to count. Dereferencing will also throw an error if no value was passed and no default was given.

The same functions as the first syntax are supported, only with `make` instead of `add`, and with the variable to bind to replaced by the default value (optional). If you want to use something other than a string option and do not want to give a default, you need to give a template parameter with the type.

Value wraps a `shared_ptr` to a `unique_ptr` to a value, so it lasts even if the `App` object is destructed.

## How it works

Every `make_` or `add_` option you've seen depends on one method that takes a lambda function. Each of these methods is just making a different lambda function with capture to populate the option. The function has full access to the vector of vector of strings, so it knows how many times an option was passed, and how many arguments each passing received (flags add empty strings to keep the counts correct). The lambda returns true if it could validate the option strings, and
false if it failed.


