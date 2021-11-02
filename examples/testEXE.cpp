#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include <iostream>
#include <map>

enum Level {
    LL_SHUTDOWN = 0,
    LL_CRITICAL = 1,
    LL_ERROR_MAPPING = 2,
    LL_ERROR = 3,
    LL_WARNING = 4,
    LL_INFO = 5,
    LL_DEBUG_NORMAL = 6,
    LL_DEBUG_DETAIL = 7,
};

std::map<std::string, Level> LogLevelMap({{"error", LL_ERROR},
                                          {"warning", LL_WARNING},
                                          {"info", LL_INFO},
                                          {"debug", LL_DEBUG_NORMAL},
                                          {"trace", LL_DEBUG_DETAIL}});

int main(int argc, const char *argv[]) {

    Level logLevel;
    int version;

    CLI::App app{"Hello App"};

    app.add_option("-v,--verbosity", logLevel, "Log/Verbosity level")
        ->transform(CLI::CheckedTransformer(LogLevelMap))
        ->default_val(LL_INFO);

    auto subcom = app.add_subcommand("subcom", "test subocmmand")->fallthrough();
    // subcom->add_option_group("Group");
    subcom->preparse_callback([&app, &version](size_t) { app.get_subcommand("subcom")->add_option_group("Group"); });

    CLI11_PARSE(app, argc, argv);

    std::cout << "Log Level: " << logLevel << std::endl;
}
