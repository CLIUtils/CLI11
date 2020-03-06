#include "app_helper.hpp"
#include "gmock/gmock.h"
#include <complex>
#include <cstdint>

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

TEST_F(TApp, BuiltinComplexFloat) {
    std::complex<float> comp{1, 2};
    app.add_complex<std::complex<float>, float>("-c,--complex", comp, "", true);

    args = {"-c", "4", "3"};

    std::string help = app.help();
    EXPECT_THAT(help, HasSubstr("1"));
    EXPECT_THAT(help, HasSubstr("2"));
    EXPECT_THAT(help, HasSubstr("COMPLEX"));

    EXPECT_FLOAT_EQ(1, comp.real());
    EXPECT_FLOAT_EQ(2, comp.imag());

    run();

    EXPECT_FLOAT_EQ(4, comp.real());
    EXPECT_FLOAT_EQ(3, comp.imag());
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

TEST_F(TApp, BuiltinComplexSingleArg) {
    cx comp{1, 2};
    app.add_complex("-c,--complex", comp);

    args = {"-c", "4"};
    run();
    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(0, comp.imag());

    args = {"-c", "4-2i"};
    run();
    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(-2, comp.imag());
    args = {"-c", "4+2i"};
    run();
    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(2, comp.imag());

    args = {"-c", "-4+2j"};
    run();
    EXPECT_DOUBLE_EQ(-4, comp.real());
    EXPECT_DOUBLE_EQ(2, comp.imag());

    args = {"-c", "-4.2-2j"};
    run();
    EXPECT_DOUBLE_EQ(-4.2, comp.real());
    EXPECT_DOUBLE_EQ(-2, comp.imag());

    args = {"-c", "-4.2-2.7i"};
    run();
    EXPECT_DOUBLE_EQ(-4.2, comp.real());
    EXPECT_DOUBLE_EQ(-2.7, comp.imag());
}

TEST_F(TApp, BuiltinComplexSingleImag) {
    cx comp{1, 2};
    app.add_complex("-c,--complex", comp);

    args = {"-c", "4j"};
    run();
    EXPECT_DOUBLE_EQ(0, comp.real());
    EXPECT_DOUBLE_EQ(4, comp.imag());

    args = {"-c", "-4j"};
    run();
    EXPECT_DOUBLE_EQ(0, comp.real());
    EXPECT_DOUBLE_EQ(-4, comp.imag());
    args = {"-c", "-4"};
    run();
    EXPECT_DOUBLE_EQ(-4, comp.real());
    EXPECT_DOUBLE_EQ(0, comp.imag());
    args = {"-c", "+4"};
    run();
    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(0, comp.imag());
}

/// Simple class containing two strings useful for testing lexical cast and conversions
class spair {
  public:
    spair() = default;
    spair(const std::string &s1, const std::string &s2) : first(s1), second(s2) {}
    std::string first{};
    std::string second{};
};
// an example of custom converter that can be used to add new parsing options
// On MSVC and possibly some other new compilers this can be a free standing function without the template
// specialization but this is compiler dependent
namespace CLI {
namespace detail {

template <> bool lexical_cast<spair>(const std::string &input, spair &output) {

    auto sep = input.find_first_of(':');
    if((sep == std::string::npos) && (sep > 0)) {
        return false;
    }
    output = {input.substr(0, sep), input.substr(sep + 1)};
    return true;
}
}  // namespace detail
}  // namespace CLI

TEST_F(TApp, custom_string_converter) {
    spair val;
    app.add_option("-d,--dual_string", val);

    args = {"-d", "string1:string2"};

    run();
    EXPECT_EQ(val.first, "string1");
    EXPECT_EQ(val.second, "string2");
}

TEST_F(TApp, custom_string_converterFail) {
    spair val;
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
template <> bool lexical_cast<std::complex<double>>(const std::string &input, std::complex<double> &output) {
    // regular expression to handle complex numbers of various formats
    static const std::regex creg(
        R"(([+-]?(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)\s*([+-]\s*(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)[ji]*)");

    std::smatch m;
    double x{0.0}, y{0.0};
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
            std::string ival = input;
            CLI::detail::trim(ival);
            worked = CLI::detail::lexical_cast(ival, x);
        }
    }
    if(worked) {
        output = cx{x, y};
    }
    return worked;
}
}  // namespace detail
}  // namespace CLI

TEST_F(TApp, AddingComplexParserDetail) {

    bool skip_tests = false;
    try {  // check if the library actually supports regex,  it is possible to link against a non working regex in the
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

/// simple class to wrap another  with a very specific type constructor and assignment operators to test out some of the
/// option assignments
template <class X> class objWrapper {
  public:
    objWrapper() = default;
    explicit objWrapper(X obj) : val_{obj} {};
    objWrapper(const objWrapper &ow) = default;
    template <class TT> objWrapper(const TT &obj) = delete;
    objWrapper &operator=(const objWrapper &) = default;
    objWrapper &operator=(objWrapper &&) = default;
    // delete all other assignment operators
    template <typename TT> void operator=(TT &&obj) = delete;

    const X &value() const { return val_; }

  private:
    X val_{};
};

// I think there is a bug with the is_assignable in visual studio 2015 it is fixed in later versions
// so this test will not compile in that compiler
#if !defined(_MSC_VER) || _MSC_VER >= 1910

static_assert(CLI::detail::is_direct_constructible<objWrapper<std::string>, std::string>::value,
              "string wrapper isn't properly constructible");

static_assert(!std::is_assignable<objWrapper<std::string>, std::string>::value,
              "string wrapper is improperly assignable");
TEST_F(TApp, stringWrapper) {
    objWrapper<std::string> sWrapper;
    app.add_option("-v", sWrapper);
    args = {"-v", "string test"};

    run();

    EXPECT_EQ(sWrapper.value(), "string test");
}

static_assert(CLI::detail::is_direct_constructible<objWrapper<double>, double>::value,
              "double wrapper isn't properly assignable");

static_assert(!CLI::detail::is_direct_constructible<objWrapper<double>, int>::value,
              "double wrapper can be assigned from int");

static_assert(!CLI::detail::is_istreamable<objWrapper<double>>::value,
              "double wrapper is input streamable and it shouldn't be");

TEST_F(TApp, doubleWrapper) {
    objWrapper<double> dWrapper;
    app.add_option("-v", dWrapper);
    args = {"-v", "2.36"};

    run();

    EXPECT_EQ(dWrapper.value(), 2.36);

    args = {"-v", "thing"};

    EXPECT_THROW(run(), CLI::ConversionError);
}

static_assert(CLI::detail::is_direct_constructible<objWrapper<int>, int>::value,
              "int wrapper is not constructible from int64");

static_assert(!CLI::detail::is_direct_constructible<objWrapper<int>, double>::value,
              "int wrapper is constructible from double");

static_assert(!CLI::detail::is_istreamable<objWrapper<int>>::value,
              "int wrapper is input streamable and it shouldn't be");

TEST_F(TApp, intWrapper) {
    objWrapper<int> iWrapper;
    app.add_option("-v", iWrapper);
    args = {"-v", "45"};

    run();

    EXPECT_EQ(iWrapper.value(), 45);
    args = {"-v", "thing"};

    EXPECT_THROW(run(), CLI::ConversionError);
}

static_assert(!CLI::detail::is_direct_constructible<objWrapper<float>, int>::value,
              "float wrapper is constructible from int");
static_assert(!CLI::detail::is_direct_constructible<objWrapper<float>, double>::value,
              "float wrapper is constructible from double");

static_assert(!CLI::detail::is_istreamable<objWrapper<float>>::value,
              "float wrapper is input streamable and it shouldn't be");

TEST_F(TApp, floatWrapper) {
    objWrapper<float> iWrapper;
    app.add_option<objWrapper<float>, float>("-v", iWrapper);
    args = {"-v", "45.3"};

    run();

    EXPECT_EQ(iWrapper.value(), 45.3f);
    args = {"-v", "thing"};

    EXPECT_THROW(run(), CLI::ConversionError);
}

#endif
/// simple class to wrap another  with a very specific type constructor to test out some of the option assignments
class dobjWrapper {
  public:
    dobjWrapper() = default;
    explicit dobjWrapper(double obj) : dval_{obj} {};
    explicit dobjWrapper(int obj) : ival_{obj} {};

    double dvalue() const { return dval_; }
    int ivalue() const { return ival_; }

  private:
    double dval_{0.0};
    int ival_{0};
};

TEST_F(TApp, dobjWrapper) {
    dobjWrapper iWrapper;
    app.add_option("-v", iWrapper);
    args = {"-v", "45"};

    run();

    EXPECT_EQ(iWrapper.ivalue(), 45);
    EXPECT_EQ(iWrapper.dvalue(), 0.0);

    args = {"-v", "thing"};

    EXPECT_THROW(run(), CLI::ConversionError);
    iWrapper = dobjWrapper{};

    args = {"-v", "45.1"};

    run();
    EXPECT_EQ(iWrapper.ivalue(), 0);
    EXPECT_EQ(iWrapper.dvalue(), 45.1);
}

/// simple class to wrap another  with a very specific type constructor and assignment operators to test out some of the
/// option assignments
template <class X> class AobjWrapper {
  public:
    AobjWrapper() = default;
    // delete all other constructors
    template <class TT> AobjWrapper(TT &&obj) = delete;
    // single assignment operator
    AobjWrapper &operator=(X val) {
        val_ = val;
        return *this;
    }
    // delete all other assignment operators
    template <typename TT> void operator=(TT &&obj) = delete;

    const X &value() const { return val_; }

  private:
    X val_{};
};

static_assert(std::is_assignable<AobjWrapper<std::uint16_t> &, std::uint16_t>::value,
              "AobjWrapper not assignable like it should be ");

TEST_F(TApp, uint16Wrapper) {
    AobjWrapper<std::uint16_t> sWrapper;
    app.add_option<AobjWrapper<std::uint16_t>, std::uint16_t>("-v", sWrapper);
    args = {"-v", "9"};

    run();

    EXPECT_EQ(sWrapper.value(), 9u);
    args = {"-v", "thing"};

    EXPECT_THROW(run(), CLI::ConversionError);

    args = {"-v", "72456245754"};

    EXPECT_THROW(run(), CLI::ConversionError);

    args = {"-v", "-3"};

    EXPECT_THROW(run(), CLI::ConversionError);
}
