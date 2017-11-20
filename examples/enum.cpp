#include <CLI/CLI.hpp>

enum Level : std::int32_t { High, Medium, Low };

int main(int argc, char **argv) {
    CLI::App app;

    Level level;
    app.add_set("-l,--level", level, {High, Medium, Low}, "Level settings")
        ->set_type_name("enum/Level in {High=0, Medium=1, Low=2}");

    CLI11_PARSE(app, argc, argv);

    return 0;
}
