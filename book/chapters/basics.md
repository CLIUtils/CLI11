# The Basics {#book-basics}

The simplest CLI11 program looks like this:

\include simplest.cpp

The first line includes the library; this uses the normal headers of the full
edition (see [Selecting an edition](@ref book-installation)). With the single
file edition you would include `CLI11.hpp` instead.

After entering the main function, you'll see that a `CLI::App` object is
created. This is the basis for all interactions with the library. You could
optionally provide a description for your app here.

A normal CLI11 application would define some flags and options next. This is a
simplest possible example, so we'll go on.

The macro `CLI11_PARSE` just runs five simple lines. This internally runs
`app.parse(argc, argv)`, which takes the command line info from C++ and parses
it. If there is an error, it throws a `ParseError`; if you catch it, you can use
`app.exit` with the error as an argument to print a nice message and produce the
correct return code for your application.

If you just use `app.parse` directly, your application will still work, but the
stack will not be correctly unwound since you have an uncaught exception, and
the command line output will be cluttered, especially for help.

For this (and most of the examples in this book) we will assume that the CLI11
`include` directory is on the compiler include path and that we are creating an
output executable `a.out` on a macOS or Linux system. (With the single file
edition, you could instead place `CLI11.hpp` next to your source code and skip
the include path.) The commands to compile and test this example would be:

```text
$ g++ -std=c++11 simplest.cpp -I/path/to/CLI11/include
$ ./a.out -h
Usage: ./a.out [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
```
