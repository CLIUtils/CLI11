// ===================================================================
//   main.cpp
// ===================================================================

#include "subcommand_a.hpp"

int main( int argc, char** argv ) {
    CLI::App app{"..."};

    // Call the setup functions for the subcommands.
    // They return their option structs, so that the variables to which the options bind
    // are still alive when `app.parse()` is called.
    auto subcommand_a_opt = setup_subcommand_a( app );

    // More setup if needed, i.e., other subcommands etc.

    CLI11_PARSE(app, argc, argv);

    return 0;
}
