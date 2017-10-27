// ===================================================================
//   subcommand_a.hpp
// ===================================================================

#include "CLI/CLI.hpp"
#include <memory>
#include <string>

/// Collection of all options of Subcommand A.
struct SubcommandAOptions {
    std::string file;
    bool with_foo;
};

// We could manually make a few variables and use shared pointers for each; this
// is just done this way to be nicely organized

// Function declarations.
void setup_subcommand_a(CLI::App &app);
void run_subcommand_a(SubcommandAOptions const &opt);
