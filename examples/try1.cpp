#include "CLI.hpp"


int main (int argc, char** argv) {

    CLI::App app("K3Pi goofit fitter");
    CLI::App* start = app.add_subcommand("start");
    CLI::App* stop = app.add_subcommand("stop");

    std::cout << app.help();
    std::string file;
    start->add_option("f,file", file, "File name");
    
    int count;
    stop->add_flag("c,count", count, "File name");

    try {
        app.run(argc, argv);
    } catch (const CLI::Error &e) {
        return app.exit(e);
    }

    std::cout << "Working on file: " << file << ", direct count: " << start->count("file") << std::endl;
    std::cout << "Working on count: " << count << ", direct count: " << stop->count("count") << std::endl;
    if(app.get_subcommand() != nullptr)
        std::cout << "Subcommand:" << app.get_subcommand()->get_name() << std::endl;

    return 0;
}
