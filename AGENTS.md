# CLI11 Agent Guide

Header-only C++11 command line parser library. CMake is the primary build
system; Meson and Bazel are also supported.

## Quick Build & Test

```bash
# Default workflow (configure + build + test)
cmake --workflow default

# Or manually
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j4
cmake --build build --target test   # or: cd build && ctest --output-on-failure
```

## Running a Single Test

Tests are individual Catch2 executables in `build/tests/`.

```bash
# Run one test executable directly
./build/tests/AppTest

# Or via CTest with a regex
ctest -R AppTest --output-on-failure
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
- `tidy` — Inherits `default`, adds `clang-tidy` with warnings-as-errors.

```bash
cmake --preset tidy
cmake --build --preset tidy
```

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
