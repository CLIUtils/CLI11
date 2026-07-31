# Using CLI11 as a C++20 module {#book-modules-example}

Module support is experimental. This page shows two complete programs that use
`import cli11;`. For the build options, targets, and requirements, see [Modules
in the installation chapter](@ref book-installation).

## A complete minimal project

Build and install CLI11 with `-DCLI11_MODULES=ON` first, then use this project:

```cmake
cmake_minimum_required(VERSION 3.28)
project(myapp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CLI11 REQUIRED)

add_executable(myapp myapp.cpp)
target_link_libraries(myapp PRIVATE CLI11::Module)
```

```cpp
// myapp.cpp
// Keep the includes before the import; GCC rejects the reverse order.
#include <iostream>
#include <string>

import cli11;

int main(int argc, char *argv[]) {
    CLI::App app{"MyApp"};

    std::string file;
    app.add_option("-f,--file", file, "The file name")->required();

    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError &e) {
        return app.exit(e);
    }

    std::cout << "file=" << file << '\n';
    return 0;
}
```

Use the same compiler for the CLI11 library and the application. Macros such as
`CLI11_PARSE` are not available through `import cli11;` — use `app.parse()` and
catch `CLI::ParseError`, or include the headers as well.

## Use with import std

You can combine `import cli11;` with `import std;`. This needs C++23 and a
standard library that ships the `std` module (recent libc++, MSVC, or libstdc++
from GCC 15+). CMake support for `import std` (`CMAKE_CXX_MODULE_STD`, CMake
3.30+) is experimental and is behind the `CMAKE_EXPERIMENTAL_CXX_IMPORT_STD`
gate, so this example invokes clang and libc++ directly:

```cpp
// myapp.cpp
import cli11;
import std;

int main(int argc, char *argv[]) {
    CLI::App app{"MyApp"};

    std::string file;
    app.add_option("-f,--file", file, "The file name")->required();

    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError &e) {
        return app.exit(e);
    }

    std::println("file={}", file);
    return 0;
}
```

```sh
# CLI11 = path to the CLI11 sources, LIBCXX = path that contains std.cppm
# (for example <llvm prefix>/share/libc++/v1)
clang++ -std=c++23 -O2 -Wno-reserved-module-identifier --precompile \
    $LIBCXX/std.cppm -o std.pcm
clang++ -std=c++23 -O2 -Wno-reserved-module-identifier -c std.pcm -o std.o
clang++ -std=c++23 -O2 -DCLI11_COMPILE -I $CLI11/include --precompile \
    $CLI11/src/modules/CLI11.cppm -o cli11.pcm
clang++ -std=c++23 -O2 -c cli11.pcm -o cli11.o
clang++ -std=c++23 -O2 -DCLI11_COMPILE -I $CLI11/include \
    -c $CLI11/src/Precompile.cpp -o impl.o
clang++ -std=c++23 -O2 -fmodule-file=std=std.pcm \
    -fmodule-file=cli11=cli11.pcm -c myapp.cpp -o myapp.o
clang++ myapp.o cli11.o impl.o std.o -o myapp
```

With both modules prebuilt, only the last two commands run again when
`myapp.cpp` changes, and they are fast; in our tests the application compiled
several times faster than an equivalent header-only build. Exact times depend on
the compiler and the application.
