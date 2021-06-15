// This file is a "Hello, world!" CLI11 program

#include "CLI/CLI.hpp"

#include <iostream>

int main(int argc, char **argv) {

    CLI::App app("Some nice description");

    int x = 0;
    app.add_option("-x", x, "an integer value")->capture_default_str();

    bool flag;
    app.add_flag("-f,--flag", flag, "a flag option");

    CLI11_PARSE(app, argc, argv);

    return 0;
}
