#ifdef CLI_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "gtest/gtest.h"

typedef std::vector<std::string> input_t;

TEST(Basic, Empty) {

    {
    CLI::App app;
    input_t simpleput;
    app.parse(simpleput);
    }
    {
    CLI::App app;
    input_t spare = {"spare"};
    EXPECT_THROW(app.parse(spare), CLI::ExtrasError);
    }
    {
    CLI::App app;
    input_t simpleput;
    app.parse(simpleput);
    }
}


