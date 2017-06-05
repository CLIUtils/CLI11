#include "CLI/CLI.hpp"

int main(int argc, char **argv) {

    CLI::App app("Prefix command app");
    app.prefix_command();

    std::vector<int> vals;
    app.add_option("--vals,-v", vals)
        ->expected(1);

    std::vector<std::string> more_comms;
    try {
        more_comms = app.parse(argc, argv);
    } catch(const CLI::ParseError &e) {
        return app.exit(e);
    }

    std::cout << "Prefix:";
    for(int v : vals)
        std::cout << v << ":";
    
    std::cout << std::endl << "Remaining commands: ";

    // Perfer to loop over from beginning, not "pop" order
    std::reverse(std::begin(more_comms), std::end(more_comms));
    for(auto com : more_comms)
        std::cout << com << " ";
    std::cout << std::endl;

    return 0;
}
