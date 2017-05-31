#include "CLI/CLI.hpp"

int main(int argc, char **argv) {

    CLI::App app("K3Pi goofit fitter");
    app.add_flag("--random", "Some random flag");
    CLI::App *start = app.add_subcommand("start", "A great subcommand");
    CLI::App *stop = app.add_subcommand("stop", "Do you really want to stop?");

    std::string file;
    start->add_option("-f,--file", file, "File name");

    CLI::Option *s = stop->add_flag("-c,--count", "Counter");

    try {
        app.parse(argc, argv);
    } catch(const CLI::Error &e) {
        return app.exit(e);
    }

    std::cout << "Working on file: " << file << ", direct count: " << start->count("--file") << std::endl;
    std::cout << "Working on count: " << s->count() << ", direct count: " << stop->count("--count") << std::endl;
    for(auto subcom : app.get_subcommands())
        std::cout << "Subcommand:" << subcom->get_name() << std::endl;

    return 0;
}
