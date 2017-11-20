#include <CLI/CLI.hpp>
#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

int main(int argc, char **argv) {
    CLI::App app;

    std::vector<int> foos;
    auto foo = app.add_option("--foo,-f", foos);

    std::vector<int> bars;
    auto bar = app.add_option("--bar", bars);

    app.add_flag("--z,--x"); // Random other flags

    // Standard parsing lines (copy and paste in, or use CLI11_PARSE)
    try {
        app.parse(argc, argv);
    } catch(const CLI::ParseError &e) {
        return app.exit(e);
    }

    // I perfer using the back and popping
    std::reverse(std::begin(foos), std::end(foos));
    std::reverse(std::begin(bars), std::end(bars));

    std::vector<std::pair<std::string, int>> keyval;
    for(auto option : app.parse_order()) {
        if(option == foo) {
            keyval.emplace_back("foo", foos.back());
            foos.pop_back();
        }
        if(option == bar) {
            keyval.emplace_back("bar", bars.back());
            bars.pop_back();
        }
    }

    // Prove the vector is correct
    for(auto &pair : keyval) {
        std::cout << pair.first << " : " << pair.second << std::endl;
    }
}
