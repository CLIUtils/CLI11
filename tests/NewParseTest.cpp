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

TEST_F(TApp, BuiltinComplexWithDelimiter) {
    cx comp{1, 2};
    app.add_complex("-c,--complex", comp, "", true)->delimiter('+');

    args = {"-c", "4+3i"};

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, HasSubstr("2"));
    EXPECT_THAT(help, HasSubstr("COMPLEX"));

    EXPECT_DOUBLE_EQ(1, comp.real());
    EXPECT_DOUBLE_EQ(2, comp.imag());

    run();

    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(3, comp.imag());

    args = {"-c", "5+-3i"};
    run();

    EXPECT_DOUBLE_EQ(5, comp.real());
    EXPECT_DOUBLE_EQ(-3, comp.imag());

    args = {"-c", "6", "-4i"};
    run();

    EXPECT_DOUBLE_EQ(6, comp.real());
    EXPECT_DOUBLE_EQ(-4, comp.imag());
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

// an example of custom converter that can be used to add new parsing options
// On MSVC and possibly some other new compilers this can be a free standing function without the template
// specialization but this is compiler dependent
namespace CLI {
namespace detail {

template <>
bool lexical_cast<std::pair<std::string, std::string>>(std::string input, std::pair<std::string, std::string> &output) {

    auto sep = input.find_first_of(':');
    if((sep == std::string::npos) && (sep > 0)) {
        return false;
    }
    output = {input.substr(0, sep), input.substr(sep + 1)};
    return true;
}
} // namespace detail
} // namespace CLI

TEST_F(TApp, custom_string_converter) {
    std::pair<std::string, std::string> val;
    app.add_option("-d,--dual_string", val);

    args = {"-d", "string1:string2"};

    run();
    EXPECT_EQ(val.first, "string1");
    EXPECT_EQ(val.second, "string2");
}

TEST_F(TApp, custom_string_converterFail) {
    std::pair<std::string, std::string> val;
    app.add_option("-d,--dual_string", val);

    args = {"-d", "string2"};

    EXPECT_THROW(run(), CLI::ConversionError);
}

// an example of custom complex number converter that can be used to add new parsing options
#if defined(__has_include)
#if __has_include(<regex>)
// an example of custom converter that can be used to add new parsing options
#define HAS_REGEX_INCLUDE
#endif
#endif

#ifdef HAS_REGEX_INCLUDE
// Gcc 4.8 and older and the corresponding standard libraries have a broken <regex> so this would
// fail.  And if a clang compiler is using libstd++ then this will generate an error as well so this is just a check to
// simplify compilation and prevent a much more complicated #if expression
#include <regex>
namespace CLI {
namespace detail {

// On MSVC and possibly some other new compilers this can be a free standing function without the template
// specialization but this is compiler dependent
template <> bool lexical_cast<std::complex<double>>(std::string input, std::complex<double> &output) {
    // regular expression to handle complex numbers of various formats
    static const std::regex creg(
        R"(([+-]?(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)\s*([+-]\s*(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)[ji]*)");

    std::smatch m;
    double x = 0.0, y = 0.0;
    bool worked;
    std::regex_search(input, m, creg);
    if(m.size() == 9) {
        worked = CLI::detail::lexical_cast(m[1], x) && CLI::detail::lexical_cast(m[6], y);
        if(worked) {
            if(*m[5].first == '-') {
                y = -y;
            }
        }
    } else {
        if((input.back() == 'j') || (input.back() == 'i')) {
            auto strval = input.substr(0, input.size() - 1);
            CLI::detail::trim(strval);
            worked = CLI::detail::lexical_cast(strval, y);
        } else {
            CLI::detail::trim(input);
            worked = CLI::detail::lexical_cast(input, x);
        }
    }
    if(worked) {
        output = cx{x, y};
    }
    return worked;
}
} // namespace detail
} // namespace CLI

TEST_F(TApp, AddingComplexParserDetail) {

    bool skip_tests = false;
    try { // check if the library actually supports regex,  it is possible to link against a non working regex in the
          // standard library
        std::smatch m;
        std::string input = "1.5+2.5j";
        static const std::regex creg(
            R"(([+-]?(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)\s*([+-]\s*(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)[ji]*)");

        auto rsearch = std::regex_search(input, m, creg);
        if(!rsearch) {
            skip_tests = true;
        } else {
            EXPECT_EQ(m.size(), 9u);
        }

    } catch(...) {
        skip_tests = true;
    }
    if(!skip_tests) {
        cx comp{0, 0};
        app.add_option("-c,--complex", comp, "add a complex number option");
        args = {"-c", "1.5+2.5j"};

        run();

        EXPECT_DOUBLE_EQ(1.5, comp.real());
        EXPECT_DOUBLE_EQ(2.5, comp.imag());
        args = {"-c", "1.5-2.5j"};

        run();

        EXPECT_DOUBLE_EQ(1.5, comp.real());
        EXPECT_DOUBLE_EQ(-2.5, comp.imag());
    }
}
#endif
