#include <CLI/CLI.hpp>

enum class Level : int { High, Medium, Low };

int main(int argc, char **argv) {
    CLI::App app;

    Level level;
    // specify string->value mappings
    std::vector<std::pair<std::string, Level>> map{
        {"high", Level::High}, {"medium", Level::Medium}, {"low", Level::Low}};
    // checked Transform does the translation and checks the results are either in one of the strings or one of the
    // translations already
    app.add_option("-l,--level", level, "Level settings")->transform(CLI::CheckedTransformer(map, CLI::ignore_case));

    CLI11_PARSE(app, argc, argv);

    return 0;
}
