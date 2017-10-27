// ===================================================================
//   subcommand_a.hpp
// ===================================================================

#include "CLI/CLI.hpp"
#include <memory>
#include <string>

/// Collection of all options of Subcommand A.
struct SubcommandAOptions
{
    std::string file;
    bool        with_foo;
};

// Function declarations.
std::unique_ptr<SubcommandAOptions> setup_subcommand_a( CLI::App& app );
void run_subcommand_a( SubcommandAOptions const& opt );
