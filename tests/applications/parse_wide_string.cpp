#include <CLI/CLI.hpp>
#include <fstream>
#include <string>

int main(int argc, char **argv) {
    CLI::App app;
    std::wstring str;

    app.add_option("str", str, "wide string");

    CLI11_PARSE(app, argc, argv);

    std::wofstream{"parse_wide_string.out.txt"} << str;
}
