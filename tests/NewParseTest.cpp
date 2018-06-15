#include "app_helper.hpp"
#include "gmock/gmock.h"
#include <complex>

using ::testing::HasSubstr;

using cx = std::complex<double>;

CLI::Option *
add_option(CLI::App &app, std::string name, cx &variable, std::string description = "", bool defaulted = false) {
    CLI::callback_t fun = [&variable](CLI::results_t res) {
        double x, y;
        bool worked = CLI::detail::lexical_cast(res[0], x) && CLI::detail::lexical_cast(res[1], y);
        if(worked)
            variable = cx(x, y);
        return worked;
    };

    CLI::Option *opt = app.add_option(name, fun, description, defaulted);
    opt->type_name("COMPLEX")->type_size(2);
    if(defaulted) {
        std::stringstream out;
        out << variable;
        opt->default_str(out.str());
    }
    return opt;
}

TEST_F(TApp, AddingComplexParser) {

    cx comp{0, 0};
    add_option(app, "-c,--complex", comp);
    args = {"-c", "1.5", "2.5"};

    run();

    EXPECT_DOUBLE_EQ(1.5, comp.real());
    EXPECT_DOUBLE_EQ(2.5, comp.imag());
}

TEST_F(TApp, DefaultComplex) {

    cx comp{1, 2};
    add_option(app, "-c,--complex", comp, "", true);
    args = {"-c", "4", "3"};

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, HasSubstr("2"));

    EXPECT_DOUBLE_EQ(1, comp.real());
    EXPECT_DOUBLE_EQ(2, comp.imag());

    run();

    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(3, comp.imag());
}

TEST_F(TApp, BuiltinComplex) {
    cx comp{1, 2};
    app.add_complex("-c,--complex", comp, "", true);

    args = {"-c", "4", "3"};

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, HasSubstr("2"));
    EXPECT_THAT(help, HasSubstr("COMPLEX"));

    EXPECT_DOUBLE_EQ(1, comp.real());
    EXPECT_DOUBLE_EQ(2, comp.imag());

    run();

    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(3, comp.imag());
}

TEST_F(TApp, BuiltinComplexIgnoreI) {
    cx comp{1, 2};
    app.add_complex("-c,--complex", comp);

    args = {"-c", "4", "3i"};

    run();

    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(3, comp.imag());
}

TEST_F(TApp, BuiltinComplexFail) {
    cx comp{1, 2};
    app.add_complex("-c,--complex", comp);

    args = {"-c", "4"};

    EXPECT_THROW(run(), CLI::ArgumentMismatch);
}
