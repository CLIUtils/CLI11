#include <CLI/CLI.hpp>
#include <iostream>

int main(int argc, char **argv) {
    CLI::App app;

    int val;
    // add a set of flags with default values associate with them
    app.add_flag("-1{1},-2{2},-3{3},-4{4},-5{5},-6{6}, -7{7}, -8{8}, -9{9}", val, "compression level");

    CLI11_PARSE(app, argc, argv);

    std::cout << "value = " << val << std::endl;
    return 0;
}
