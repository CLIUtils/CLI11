# Unicode support {#book-unicode}

CLI11 follows the [UTF-8 Everywhere](http://utf8everywhere.org/) manifesto:
strings are UTF-8 everywhere inside the library, and wide strings are converted
at the edges.

Three things follow from this:

- CLI11 can parse the wide form of the command line on Windows and convert it to
  UTF-8 internally.
- An option value can be a `std::wstring`. CLI11 converts it to the correct wide
  encoding for your system, UTF-16 on Windows and UTF-32 on most others.
- Rather than store wide strings, it is better to keep `std::string` and convert
  only when you must, such as when you call a Windows API.

## Getting correct arguments on Windows

On Windows the `argv` that reaches `main` may already have lost information,
because the arguments were converted to the local code page. Parsing that `argv`
cannot give you the right string back.

The recommended fix works on every platform:

```cpp
int main(int argc, char **argv) {
    CLI::App app;
    argv = app.ensure_utf8(argv);  // new argv memory is held by app
    // ...
    CLI11_PARSE(app, argc, argv);
}
```

On Linux and macOS `ensure_utf8` returns `argv` unchanged. On Windows it
discards `argv` and rebuilds it from the win32 API. Call it before you read or
change `argv`, since the values are reconstructed and any earlier change is
lost.

Two alternatives exist. The first is the Windows-only `wmain` function, which
gets `wchar_t *argv[]`; CLI11 accepts wide arguments directly:

```cpp
int wmain(int argc, wchar_t *argv[]) {
    CLI::App app;
    // ...
    CLI11_PARSE(app, argc, argv);
}
```

The second is to get the arguments yourself with a Windows API such as
`CommandLineToArgvW` and pass them to CLI11. That is what `ensure_utf8` does
internally.

## Converting between narrow and wide strings

```cpp
namespace CLI {
    std::string narrow(const std::wstring &str);
    std::string narrow(const wchar_t *str);
    std::string narrow(const wchar_t *str, std::size_t size);
    std::string narrow(std::wstring_view str);  // C++17

    std::wstring widen(const std::string &str);
    std::wstring widen(const char *str);
    std::wstring widen(const char *str, std::size_t size);
    std::wstring widen(std::string_view str);  // C++17
}
```

## Unicode paths

A `std::filesystem::path` on Windows must be built from a wide string, or the
name is mangled. `CLI::to_path` does the correct conversion on every platform:

```cpp
std::string utf8_name = "Hello Halló Привет 你好 👩‍🚀❤️.txt";

std::filesystem::path p = CLI::to_path(utf8_name);
std::ifstream stream(CLI::to_path(utf8_name));
```

`to_path` needs `<filesystem>` support (C++17, `CLI11_HAS_FILESYSTEM`).
