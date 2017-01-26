
#include "CLI.hpp"
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
    EXPECT_THROW(app.parse(spare), CLI::ExtraPositionalsError);
    }
    {
    CLI::App app;
    input_t simpleput;
    app.parse(simpleput);
    }
}

struct TApp : public ::testing::Test {
    CLI::App app{"My Test Program"};
    input_t args;

    void run() {
        input_t newargs = args;
        std::reverse(std::begin(newargs), std::end(newargs));
        app.parse(newargs);
    }

};

TEST_F(TApp, AFewArgs) {
    app.add_flag("c,count");
    args = {"-c"};
    run();
    EXPECT_EQ(1, app.count("count"));
}
