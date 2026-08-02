# Installation {#book-installation}

## Single file edition

```cpp
#include <CLI11.hpp>
```

This example uses the single file edition of CLI11. You can download `CLI11.hpp`
from the latest release and put it into the same folder as your source code,
then compile this with C++ enabled. For a larger project, you can just put this
in an include folder and you are set. This is the simplest and most
straightforward means of including CLI11 with a project.

## Full edition

```cpp
#include <CLI/CLI.hpp>
```

If you want to use CLI11 in its full form, you can also use the original
multiple file edition. This has an extra utility (`Timer`), and is does not
require that you use a release. The only change to your code would be the
include shown above.

### CMake support for the full edition

If you use CMake 3.14+ for your project (highly recommended), CLI11 comes with a
powerful CMakeLists.txt file that was designed to also be used with
`add_subdirectory`. You can add the repository to your code (preferably as a git
submodule), then add the following line to your project (assuming your folder is
called CLI11):

```cmake
add_subdirectory(CLI11)
```

Then, you will have a target `CLI11::CLI11` that you can link to with
`target_link_libraries`. It will provide the include paths you need for the
library. This is the way [GooFit](https://github.com/GooFit/GooFit) uses CLI11,
for example.

You can also configure and optionally install CLI11, and CMake will create the
necessary `lib/cmake/CLI11/CLI11Config.cmake` files, so
`find_package(CLI11 CONFIG REQUIRED)` also works.

If you use conan.io, CLI11 supports that too. CLI11 also supports Meson and
pkg-config if you are not using CMake.

#### Precompiled mode

CLI11 is header-only by default: every function is `inline`, so each translation
unit that includes CLI11 compiles the whole library again. In a large project
that includes CLI11 in many places, this is slow.

Set the CMake option `CLI11_PRECOMPILED` to compile the library one time into a
static library instead:

```bash
cmake -S . -B build -DCLI11_PRECOMPILED=ON
```

The target you link against does not change. `CLI11::CLI11` is a static library
instead of an interface library, and it applies the `CLI11_COMPILE` definition
to your code for you:

```cmake
target_link_libraries(MyTarget PRIVATE CLI11::CLI11)
```

The mechanism is the `CLI11_INLINE` macro. Each public header includes a
matching `CLI/impl/*_inl.hpp` implementation header at the end. Without
`CLI11_COMPILE`, `CLI11_INLINE` expands to `inline` and the implementation is
part of the header. With `CLI11_COMPILE`, `CLI11_INLINE` expands to nothing, the
headers stop including the implementation, and the definitions must come from
somewhere else.

If you do not use CLI11's CMake, you can do the same thing yourself. Define
`CLI11_COMPILE` for all of your code, then compile one source file that includes
the implementation headers:

```cpp
// cli11_impl.cpp - compiled one time
#include <CLI/impl/App_inl.hpp>
#include <CLI/impl/Argv_inl.hpp>
#include <CLI/impl/Config_inl.hpp>
#include <CLI/impl/Encoding_inl.hpp>
#include <CLI/impl/ExtraValidators_inl.hpp>
#include <CLI/impl/Formatter_inl.hpp>
#include <CLI/impl/Option_inl.hpp>
#include <CLI/impl/Split_inl.hpp>
#include <CLI/impl/StringTools_inl.hpp>
#include <CLI/impl/Validators_inl.hpp>
```

This is exactly what `src/Precompile.cpp` does in the CLI11 repository.

Two limits apply. `CLI11_PRECOMPILED` and `CLI11_SINGLE_FILE` are mutually
exclusive, because the single header is header-only by construction. Also, the
`impl` headers must be installed for a precompiled build to be usable from an
install tree; set `CLI11_DISABLE_IMPL_HEADERS_INSTALL` only if you do not need
them.

#### Global Headers

Use `CLI/*.hpp` files stored in a shared folder. You could check out the git
repository to a system-wide folder, for example `/opt/`. With CMake, you could
add to the include path via:

```bash
if(NOT DEFINED CLI11_DIR)
set (CLI11_DIR "/opt/CLI11" CACHE STRING "CLI11 git repository")
endif()
include_directories(${CLI11_DIR}/include)
```

And then in the source code (adding several headers might be needed to prevent
linker errors):

```cpp
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"
```

#### Global Headers with Target

Configuring and installing the project is required for linking CLI11 to your
project in the same way as you would do with any other external library. With
CMake, this step allows using `find_package(CLI11 CONFIG REQUIRED)` and then
using the `CLI11::CLI11` target when linking. If `CMAKE_INSTALL_PREFIX` was
changed during install to a specific folder like `/opt/CLI11`, then you have to
pass `-DCLI11_DIR=/opt/CLI11` when building your current project. You can also
use [Conan.io](https://conan.io/center/cli11) or
[Hunter](https://docs.hunter.sh/en/latest/packages/pkg/CLI11.html). (These are
just conveniences to allow you to use your favorite method of managing packages;
it's just header only so including the correct path and using C++11 is all you
really need.)

#### Modules

Module support is experimental. To use modules, you must use C++20 or later,
CMake 3.28 or later, and the Ninja or Visual Studio generator (Makefiles do not
work). Build and install CLI11 with `-DCLI11_MODULES=ON`, then link the target
`CLI11::Module` (the older name `CLI11::CLI11_Module` stays supported). The
module library contains the precompiled implementation, so an `import cli11;`
client usually compiles several times faster than one that includes the headers.
Macros such as `CLI11_PARSE` are not available through `import cli11;` — use
`app.parse()` and catch `CLI::ParseError`, or include the headers as well. For
complete programs, including an `import std;` variant, see [Using CLI11 as a
C++20 module](@ref book-modules-example).

#### Using Fetchcontent

If you do not want to add cmake as a submodule or include it with your code the
project can be added using `FetchContent`. This capability requires CMake 3.14+
(or 3.11+ with more work).

An example CMake file would include:

```cmake
include(FetchContent)
FetchContent_Declare(
    cli11_proj
    QUIET
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.7.2
)

FetchContent_MakeAvailable(cli11_proj)

# And now you can use it
target_link_libraries(<your project> PRIVATE CLI11::CLI11)
```

And use

```c++
#include <CLI/CLI.hpp>
```

in your project. It is highly recommended that you use the git hash for
`GIT_TAG` instead of a tag or branch, as that will both be more secure, as well
as faster to reconfigure - CMake will not have to reach out to the internet to
see if the tag moved. You can also download just the single header file from the
releases using `file(DOWNLOAD)`.

### Running tests on the full edition

CLI11 has examples and tests that can be accessed using a CMake build on any
platform. Simply build and run ctest to run the 200+ tests to ensure CLI11 works
on your system.

The test build uses Catch2. If Catch2 is not available as a CMake package, CLI11
will download the required Catch2 header during configuration. On systems
without internet access, or where TLS connections to GitHub releases are
blocked, install Catch2 separately or configure with `CLI11_BUILD_TESTS=OFF`
until the dependency is available.

As an example of the build system, the following commands will download and test
CLI11 in a simple Alpine Linux docker container. (Docker is being used to create
a pristine disposable environment; there is nothing special about this
container. Alpine is being used because it is small, modern, and fast. Commands
are similar on any other platform.)

```bash
docker run -it alpine
apk add --no-cache g++ cmake make git
git clone https://github.com/CLIUtils/CLI11.git
cd CLI11
mkdir build
cd build
cmake ..
make
make test
```

For the curious, the CMake options and defaults are listed below. Most options
default to off if CLI11 is used as a subdirectory in another project.

| Option                               | Description                                                      |
| ------------------------------------ | ---------------------------------------------------------------- |
| `CLI11_SINGLE_FILE=OFF`              | Build the `CLI11.hpp` file from the sources. Requires Python.    |
| `CLI11_PRECOMPILED=OFF`              | Generate a precompiled library instead of header-only            |
| `CLI11_MODULES=OFF`                  | Build CLI11 as a module (requires C++20 or later)                |
| `CLI11_INSTALL_PACKAGE_TESTS=OFF`    | Run tests checking the installation                              |
| `CLI11_MODULE_TESTS=OFF`             | Run a test checking that CLI11 works with modules                |
| `CLI11_SINGLE_FILE_TESTS=OFF`        | Run the tests on the generated single file version as well       |
| `CLI11_BUILD_DOCS=ON`                | Build CLI11 documentation and book                               |
| `CLI11_BUILD_EXAMPLES=ON`            | Build the example programs.                                      |
| `CLI11_BUILD_EXAMPLES_JSON=ON`       | Build some additional example using json libraries               |
| `CLI11_INSTALL=ON`                   | Install CLI11 to the install folder during the install process   |
| `CLI11_FULL_INSTALL=OFF`             | Install all CLI11 headers/libraries regardless of other settings |
| `CLI11_FORCE_LIBCXX=OFF`             | Use libc++ instead of libstdc++ if building with clang on linux  |
| `CLI11_DISABLE_IMPL_HEADERS_INSTALL` | Don't install the impl headers if the CLI11_PRECOMPILED is ON    |
| `CLI11_CUDA_TESTS=OFF`               | Build the tests with NVCC                                        |
| `CLI11_BUILD_TESTS=ON`               | Build the tests.                                                 |
| `CLI11_ENABLE_EXTRA_VALIDATORS`      | Set to 1 to enable the extra validators, 0 to disable them       |
| `CLI11_DISABLE_EXTRA_VALIDATORS`     | Set to 1 to disable the extra validators                         |

The last two options set the macro of the same name on the CLI11 target. See
[Validators](@ref book-validators) for what they control.

## Meson support

### Global Headers from pkg-config

If CLI11 is installed globally, then nothing more than `dependency('CLI11')` is
required. If it installed in a non-default search path, then setting the
`PKG_CONFIG_PATH` environment variable of the `--pkg-config-path` option to
`meson setup` is all that's required.

### Using Meson's subprojects

Meson has a system called
[wraps](https://mesonbuild.com/Wrap-dependency-system-manual.html), which allow
Meson to fetch sources, configure, and build dependencies as part of a main
project. This is the mechanism that Meson recommends for projects to use, as it
allows updating the dependency transparently, and allows packagers to have fine
grained control on the use of subprojects vs system provided dependencies.
Simply run `meson wrap install cli11` to install the `cli11.wrap` file, and
commit it, if desired.

It is also possible to use git submodules. This is generally discouraged by
Meson upstream, but may be appropriate if a project needs to build with multiple
build systems and wishes to share subprojects between them. As long as the
submodule is in the parent project's subproject directory nothing additional is
needed.

## Bazel support

CLI11 is a Bazel module. Add it to your `MODULE.bazel`:

```python
bazel_dep(name = "cli11", version = "2.6.2")
```

Then depend on the `@cli11//:cli11` target:

```python
cc_binary(
    name = "my_app",
    srcs = ["my_app.cpp"],
    deps = ["@cli11//:cli11"],
)
```

The module builds CLI11 in precompiled mode. Bazel 7.4 or later is required.

## Installing cli11 using vcpkg

You can download and install cli11 using the
[vcpkg](https://github.com/Microsoft/vcpkg) dependency manager:

```bash
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install cli11
```

The cli11 port in vcpkg is kept up to date by Microsoft team members and
community contributors. If the version is out of date, please
[create an issue or pull request](https://github.com/Microsoft/vcpkg) on the
vcpkg repository.

## Installing CLI11 using Conan

You can install pre-built binaries for CLI11 or build it from source using
[Conan](https://conan.io/). Use the following command:

```bash
conan install --requires="cli11/[*]" --build=missing
```

The CLI11 Conan recipe is kept up to date by Conan maintainers and community
contributors. If the version is out of date, please
[create an issue or pull request](https://github.com/conan-io/conan-center-index)
on the ConanCenterIndex repository.

## Special instructions for GCC 8, Some clang, and WASI

If you are using GCC 8 and using it in C++17 mode with CLI11. CLI11 makes use of
the `<filesystem>` header if available, but specifically for this compiler, the
`filesystem` library is separate from the standard library and needs to be
linked separately. So it is available but CLI11 doesn't use it by default.

Specifically `libstdc++fs` needs to be added to the linking list and
`CLI11_HAS_FILESYSTEM=1` has to be defined. Then the filesystem variant of the
Validators could be used on GCC 8. GCC 9+ does not have this issue so the
`<filesystem>` is used by default.

There may also be other cases where a specific library needs to be linked.

Defining `CLI11_HAS_FILESYSTEM=0` which will remove the usage and hence any
linking issue.

In some cases certain clang compilations may require linking against `libc++fs`.
These situations have not been encountered so the specific situations requiring
them are unknown yet.

If building with WASI it is necessary to add the flag
`-lc-printscan-long-double` to the build to allow long double support. See #841
for more details.

## Default system packages on Linux

If you are not worried about latest features or recent bug fixes, you can
install a stable version of CLI11 using:

`sudo apt install libcli11-dev` for Ubuntu, or: `sudo dnf install cli11-devel`
on Fedora/Almalinux.

Then, in your CMake project, just call:

```cmake
find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(MyTarget PRIVATE CLI11::CLI11)
```

and in your C++ file:

```cpp
#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

int main(int argc, char** argv) {
    CLI::App app{"MyApp"};
    // Here your flags / options
    CLI11_PARSE(app, argc, argv);
}
```
