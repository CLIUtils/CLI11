// ===================================================================
//   subcommand_a.cpp
// ===================================================================

#include "subcommand_a.hpp"

/// Set up a subcommand and return a unique ptr to a struct that holds all its options.
/// The variables of the struct are bound to the CLI options.
/// We use a unique ptr so that the addresses of the variables are stable for binding,
/// and return it to the caller (main), so that the object itself stays alive.
std::unique_ptr<SubcommandAOptions> setup_subcommand_a( CLI::App& app )
{
    // Create the option and subcommand objects.
    auto opt = std::unique_ptr<SubcommandAOptions>(new SubcommandAOptions());
    auto sub = app.add_subcommand( "subcommand_a", "performs subcommand a", true );

    // Add options to sub, binding them to opt.
    sub->add_option("-f,--file", opt->file, "File name");
    sub->add_flag("--with-foo", opt->with_foo, "Counter");

    // Set the run function as callback to be called when this subcommand is issued.
    sub->set_callback( [&]() {
        run_subcommand_a( *opt );
    });

    return opt;
}

/// The function that runs our code.
/// This could also simply be in the callback lambda itself,
/// but having a separate function is cleaner.
void run_subcommand_a( SubcommandAOptions const& opt )
{
    // Do stuff...
    std::cout << "Working on file: " << opt.file << std::endl;
    if( opt.with_foo ) {
        std::cout << "Using foo!" << std::endl;
    }
}
