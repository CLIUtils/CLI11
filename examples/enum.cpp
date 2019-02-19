#include <CLI/CLI.hpp>
#include <map>
#include <sstream>

enum class Level : int { High, Medium, Low };

std::istream &operator>>(std::istream &in, Level &level) {
    int i;
    in >> i;
    level = static_cast<Level>(i);
    return in;
}

std::ostream &operator<<(std::ostream &in, const Level &level) { return in << static_cast<int>(level); }

int main(int argc, char **argv) {
    CLI::App app;

    std::map<Level, std::string> map = {{Level::High, "High"}, {Level::Medium, "Medium"}, {Level::Low, "Low"}};

    Level level;

    app.add_option("-l,--level", level, "Level settings")
        ->required()
        ->check(CLI::IsMember(map, CLI::ignore_case) | CLI::IsMember({Level::High, Level::Medium, Level::Low}));

    CLI11_PARSE(app, argc, argv);

    std::cout << "Enum received: " << level << std::endl;

    return 0;
}
