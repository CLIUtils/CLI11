#ifdef CLI11_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "gtest/gtest.h"

TEST(Deprecated, SetFooter) {
    CLI::App app{"My prog"};

    app.set_footer("My Footer");
    EXPECT_EQ("My Footer", app.get_footer());
}

TEST(Deprecated, SetName) {
    CLI::App app{"My prog"};

    app.set_name("My Name");
    EXPECT_EQ("My Name", app.get_name());
}

TEST(Deprecated, SetCallback) {
    CLI::App app{"My prog"};

    bool val;
    app.set_callback([&val]() { val = true; });

    std::vector<std::string> something;
    app.parse(something);

    EXPECT_TRUE(val);
}

TEST(Deprecated, SetTypeName) {
    CLI::App app{"My prog"};

    std::string val;
    auto opt = app.add_option("--val", val);
    opt->set_type_name("THAT");

    EXPECT_EQ(opt->get_type_name(), "THAT");
}
