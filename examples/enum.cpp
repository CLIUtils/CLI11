#include <CLI/CLI.hpp>
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

    Level level;
    app.add_set("-l,--level", level, {Level::High, Level::Medium, Level::Low}, "Level settings")
        ->type_name("enum/Level in {High=0, Medium=1, Low=2}");

    CLI11_PARSE(app, argc, argv);

    return 0;
}
