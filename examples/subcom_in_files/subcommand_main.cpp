// ===================================================================
//   main.cpp
// ===================================================================

#include "subcommand_a.hpp"

int main(int argc, char **argv) {
    CLI::App app{"..."};

    // Call the setup functions for the subcommands.
    // They are kept alive by a shared pointer in the
    // lambda function held by CLI11
    setup_subcommand_a(app);

    // Make sure we get at least one subcommand
    app.require_subcommand();

    // More setup if needed, i.e., other subcommands etc.

    CLI11_PARSE(app, argc, argv);

    return 0;
}
