#include "CLI/CLI.hpp"

int main(int argc, char **argv) {

    CLI::App app("Prefix command app");
    app.prefix_command();

    std::vector<int> vals;
    app.add_option("--vals,-v", vals)->expected(-1);

    CLI11_PARSE(app, argc, argv);

    std::vector<std::string> more_comms = app.remaining();

    std::cout << "Prefix";
    for(int v : vals)
        std::cout << ": " << v << " ";

    std::cout << std::endl << "Remaining commands: ";

    for(auto com : more_comms)
        std::cout << com << " ";
    std::cout << std::endl;

    return 0;
}
