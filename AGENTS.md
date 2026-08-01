# CLI11 Agent Guide

Header-only C++11 command line parser library. CMake is the primary build
system; Meson and Bazel are also supported.

## Quick Build & Test

Use presets. The `dev` workflow is the fastest for iteration; use `default`
before a push to verify the primary header-only mode.

```bash
# Fast iteration (precompiled lib, no examples, ccache; configure + build + test)
cmake --workflow dev

# Full header-only build, matches CI (configure + build + test)
cmake --workflow default

# Or step by step
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

The `dev` preset uses `ccache`; install it (`brew install ccache`) or override
with `cmake --preset dev -DCMAKE_CXX_COMPILER_LAUNCHER=`.

## Running a Single Test

Tests are individual Catch2 executables in `build-dev/tests/` (`dev` preset) or
`build/tests/` (`default` preset).

```bash
# Run one test executable directly
./build-dev/tests/AppTest

# Or via CTest with a regex
ctest --preset dev -R AppTest
```

## Key CMake Options

| Option                     | Default                 | Purpose                                 |
| -------------------------- | ----------------------- | --------------------------------------- |
| `CLI11_BUILD_TESTS`        | `ON` (if top-level)     | Build Catch2 test suite                 |
| `CLI11_BUILD_EXAMPLES`     | `ON` (if top-level)     | Build `examples/`                       |
| `CLI11_BUILD_DOCS`         | `ON` (if Doxygen found) | Build Doxygen docs                      |
| `CLI11_SINGLE_FILE`        | `OFF`                   | Generate single `CLI11.hpp` header      |
| `CLI11_PRECOMPILED`        | `OFF`                   | Build static lib instead of header-only |
| `CLI11_WARNINGS_AS_ERRORS` | `OFF`                   | Turn warnings into errors               |
| `CLI11_SANITIZERS`         | `OFF`                   | Enable ASan/TSan/UBSan                  |
| `CLI11_BOOST`              | `OFF`                   | Enable Boost.Optional tests             |
| `CLI11_CUDA_TESTS`         | `OFF`                   | Compile tests with NVCC                 |

`CLI11_SINGLE_FILE` and `CLI11_PRECOMPILED` are mutually exclusive.

## Presets

- `default` — Debug, Ninja, `CLI11_WARNINGS_AS_ERRORS=ON`, export compile
  commands.
- `dev` — Inherits `default`, adds `CLI11_PRECOMPILED=ON`,
  `CLI11_BUILD_EXAMPLES=OFF`, and `ccache`. An edit to `impl/*_inl.hpp` only
  rebuilds the static library, not every test.
- `tidy` — Inherits `default`, adds `clang-tidy` with warnings-as-errors. Uses
  precompiled mode, so each `impl/*_inl.hpp` header is analyzed once (in
  `src/Precompile.cpp`) instead of in every test and example.
- `iwyu` — Inherits `default`, runs `include-what-you-use`. Also precompiled,
  with tests and examples off, so `src/Precompile.cpp` is the only translation
  unit and each header is reported once.

```bash
cmake --preset tidy
cmake --build --preset tidy
```

## Include-what-you-use

`brew install include-what-you-use`, then:

```bash
cmake --preset iwyu
cmake --build --preset iwyu
```

The build always succeeds; IWYU writes its advice to stderr. Nothing enforces
it, so read the report and apply what is correct.

Two things keep the report readable:

- `scripts/iwyu.imp` maps the headers each standard library makes IWYU ask for
  to the C++ header CLI11 should use. libc++ (macOS) exposes private detail
  headers such as `<_stdlib.h>`; libstdc++ (Linux) exposes the C headers behind
  `<cstdlib>`. Read the comment at the top before you add an entry: the
  visibility on the left must agree with what IWYU already believes, or the run
  aborts with a visibility assertion.
- `--keep=*/CLI/*.hpp` holds the project's own includes. The
  `IWYU pragma: private, include "CLI/CLI.hpp"` line at the top of each header
  is for users of the library; inside the library IWYU applies it too and asks
  every header to include `CLI/CLI.hpp` in place of its siblings. `--keep` drops
  that advice and leaves the standard-library advice, which is what matters for
  the single-header build. One `should add #include "CLI/CLI.hpp"` line per
  header survives; ignore it.

The mapping file covers both standard libraries, so check the other one after
you change it:

```bash
docker run --rm -v "$PWD:/src:ro" debian:trixie sh -c '
  apt-get update -qq && apt-get install -y -qq iwyu cmake ninja-build g++ &&
  cp -r /src /work && rm -rf /work/build* && cd /work &&
  cmake --preset iwyu >/dev/null && cmake --build --preset iwyu'
```

Known false positives, all in the "should add" lists: `<version>` for
`Macros.hpp` on both platforms, plus `<AvailabilityInternal.h>` for `Macros.hpp`
and `<math>` for `Validators.hpp` on macOS. The "should remove" lines for
forward declarations are also wrong — the declarations exist to fix the order in
the single header.

## Single Header Generation

Requires Python. Enable with `CLI11_SINGLE_FILE=ON`:

```bash
cmake -S . -B build -DCLI11_SINGLE_FILE=ON
cmake --build build --target CLI11-generate-single-file
# Output: build/single-include/CLI11.hpp
```

Script: `scripts/MakeSingleHeader.py`.

## Library Structure

- `include/CLI/` — Public headers. The umbrella header is `CLI.hpp`.
- `include/CLI/impl/` — `_inl.hpp` implementation headers included by the main
  headers.
- `src/` — `.cpp` files used **only** when `CLI11_PRECOMPILED=ON`.
- `single-include/` — CMake rules for the single-header build.
- `tests/` — Catch2 tests. `main.cpp` + `catch.hpp` provide the test runner.
- `tests/data/` — Test data files copied to the build dir automatically.
- `examples/` — Standalone example programs.
- `book/` — Extra documentation/examples built only when top-level.

## Testing Notes

- Catch2 is auto-downloaded (v2.13.10 header) if not found on the system. Both
  Catch2 v2 and v3 are supported.
- Some tests launch helper applications (`ensure_utf8`, `ensure_utf8_twice`)
  built from `tests/applications/`.
- `FuzzFailTest` requires C++17.
- `WindowsTest` is only built on Windows.
- `DeprecatedTest` compiles with `-Wno-deprecated-declarations`.
- `TimerTest` is in `CLI11_MULTIONLY_TESTS` (exercises multi-threading).

## Code Style & Linting

Pre-commit hooks are configured in `.pre-commit-config.yaml`:

- `clang-format` for C++/C/CUDA
- `cmake-format` for CMake
- `black` for Python
- `prettier` for YAML/Markdown/JSON/etc.
- `codespell` for typos
- `markdownlint-cli2`
- Custom checks: disallow a few common mistakes Run locally:

```bash
prek -a
```

## Version Source of Truth

The version string is read from `include/CLI/Version.hpp` at configure time. Do
not edit project version in `CMakeLists.txt`.
