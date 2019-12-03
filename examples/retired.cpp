#include "CLI/CLI.hpp"
#include <vector>

// This example show the usage of the retired and deprecated option helper methods
int main(int argc, char **argv) {

    CLI::App app("example for retired/deprecated options");
    std::vector<int> x;
    auto opt1 = app.add_option("--retired_option2", x);

    std::pair<int, int> y;
    auto opt2 = app.add_option("--deprecate", y);

    app.add_option("--not_deprecated", x);

    // specify that a no existing option is retired
    CLI::retire_option(app, "--retired_option");

    // specify that an existing option is retired and non-functional this will replace the option with another that
    // behaves the same but does nothing
    CLI::retire_option(app, opt1);

    // deprecate an existing option and specify a recommended replacement
    CLI::deprecate_option(opt2, "--not_deprecated");

    CLI11_PARSE(app, argc, argv);
}
