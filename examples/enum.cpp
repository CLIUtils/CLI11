#include <CLI/CLI.hpp>
#include <map>
#include <sstream>

enum class Level : int { High, Medium, Low };

int main(int argc, char **argv) {
    CLI::App app;

    std::map<std::string, Level> map = {{"High", Level::High}, {"Medium", Level::Medium}, {"Low", Level::Low}};

    Level level;

    app.add_option("-l,--level", level, "Level settings")
        ->required()
        ->transform(CLI::IsMember(map, CLI::ignore_case) | CLI::IsMember({Level::High, Level::Medium, Level::Low}));

    CLI11_PARSE(app, argc, argv);

    // CLI11's built in enum streaming can be used outside CLI11 like this:
    using namespace CLI::enums;
    std::cout << "Enum received: " << level << std::endl;

    return 0;
}
