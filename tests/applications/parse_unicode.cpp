#include <CLI/CLI.hpp>
#include <fstream>
#include <string>

int main(int argc, char **argv) {
    CLI::App app;
    std::string str;

    app.add_option("str", str, "utf-8 string");

    CLI11_PARSE(app, argc, argv);

    std::ofstream{"parse_unicode.out.txt"} << str;
}
