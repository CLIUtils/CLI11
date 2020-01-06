# Installation

## Single file edition

```cpp
#include <CLI11.hpp>
```

This example uses the single file edition of CLI11. You can download `CLI11.hpp` from the latest release and put it into the same folder as your source code, then compile this with C++ enabled. For a larger project, you can just put this in an include folder and you are set.

## Full edition

```cpp
#include <CLI/CLI.hpp>
```

If you want to use CLI11 in its full form, you can also use the original multiple file edition. This has an extra utility (`Timer`), and is does not require that you use a release. The only change to your code would be the include shown above.

### CMake support for the full edition

If you use CMake 3.4+ for your project (highly recommended), CLI11 comes with a powerful CMakeLists.txt file that was designed to also be used with `add_subproject`. You can add the repository to your code (preferably as a git submodule), then add the following line to your project (assuming your folder is called CLI11):

```cmake
add_subdirectory(CLI11)
```

Then, you will have a target `CLI11::CLI11` that you can link to with `target_link_libraries`. It will provide the include paths you need for the library. This is the way [GooFit](https://github.com/GooFit/GooFit) uses CLI11, for example.

You can also configure and optionally install CLI11, and CMake will create the necessary `lib/cmake/CLI11/CLI11Config.cmake` files, so `find_package(CLI11 CONFIG REQUIRED)` also works.

If you use conan.io, CLI11 supports that too.

### Running tests on the full edition

CLI11 has examples and tests that can be accessed using a CMake build on any platform. Simply build and run ctest to run the 200+ tests to ensure CLI11 works on your system.

As an example of the build system, the following code will download and test CLI11 in a simple Alpine Linux docker container [^1]:

```term
gitbook:~ $ docker run -it alpine
root:/ # apk add --no-cache g++ cmake make git
fetch ...
root:/ # git clone https://github.com/CLIUtils/CLI11.git
Cloning into 'CLI11' ...
root:/ # cd CLI11
root:CLI11 # mkdir build
root:CLI11 # cd build
root:build # cmake ..
-- The CXX compiler identification is GNU 6.3.0 ...
root:build # make
Scanning dependencies ...
root:build # make test
[warning]Running tests...
Test project /CLI11/build
      Start  1: HelpersTest
 1/10 Test  #1: HelpersTest ......................   Passed    0.01 sec
      Start  2: IniTest
 2/10 Test  #2: IniTest ..........................   Passed    0.01 sec
      Start  3: SimpleTest
 3/10 Test  #3: SimpleTest .......................   Passed    0.01 sec
      Start  4: AppTest
 4/10 Test  #4: AppTest ..........................   Passed    0.02 sec
      Start  5: CreationTest
 5/10 Test  #5: CreationTest .....................   Passed    0.01 sec
      Start  6: SubcommandTest
 6/10 Test  #6: SubcommandTest ...................   Passed    0.01 sec
      Start  7: HelpTest
 7/10 Test  #7: HelpTest .........................   Passed    0.01 sec
      Start  8: NewParseTest
 8/10 Test  #8: NewParseTest .....................   Passed    0.01 sec
      Start  9: TimerTest
 9/10 Test  #9: TimerTest ........................   Passed    0.24 sec
      Start 10: link_test_2
10/10 Test #10: link_test_2 ......................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 10

Total Test time (real) =   0.34 sec
```

For the curious, the CMake options and defaults are listed below. Most options default to off if CLI11 is used as a subdirectory in another project.

| Option | Description |
|--------|-------------|
| `CLI11_SINGLE_FILE=ON` | Build the `CLI11.hpp` file from the sources. Requires Python (version 3 or 2.7). |
| `CLI11_SINGLE_FILE_TESTS=OFF` | Run the tests on the generated single file version as well |
| `CLI11_EXAMPLES=ON` | Build the example programs. |
| `CLI11_TESTING=ON` | Build the tests. |
| `CLI11_CLANG_TIDY=OFF` | Run `clang-tidy` on the examples and headers. Requires CMake 3.6+. |
| `CLI11_CLANG_TIDY_OPTIONS=""` | Options to pass to `clang-tidy`, such as `-fix` (single threaded build only if applying fixes!) |

[^1]: Docker is being used to create a pristine disposable environment; there is nothing special about this container. Alpine is being used because it is small, modern, and fast. Commands are similar on any other platform.
