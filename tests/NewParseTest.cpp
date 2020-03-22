// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"
#include "gmock/gmock.h"
#include <complex>
#include <cstdint>

using ::testing::HasSubstr;

using cx = std::complex<double>;

TEST_F(TApp, Complex) {
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

TEST_F(TApp, ComplexOption) {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp, "", true);

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

TEST_F(TApp, ComplexFloat) {
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

TEST_F(TApp, ComplexFloatOption) {
    std::complex<float> comp{1, 2};
    app.add_option("-c,--complex", comp, "", true);

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

TEST_F(TApp, ComplexWithDelimiter) {
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

TEST_F(TApp, ComplexWithDelimiterOption) {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp, "", true)->delimiter('+');

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

TEST_F(TApp, ComplexIgnoreI) {
    cx comp{1, 2};
    app.add_complex("-c,--complex", comp);

    args = {"-c", "4", "3i"};

    run();

    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(3, comp.imag());
}

TEST_F(TApp, ComplexIgnoreIOption) {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp);

    args = {"-c", "4", "3i"};

    run();

    EXPECT_DOUBLE_EQ(4, comp.real());
    EXPECT_DOUBLE_EQ(3, comp.imag());
}

TEST_F(TApp, ComplexSingleArg) {
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

TEST_F(TApp, ComplexSingleArgOption) {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp);

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

TEST_F(TApp, ComplexSingleImag) {
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

TEST_F(TApp, ComplexSingleImagOption) {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp);

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

template <class T> class SimpleWrapper {
  public:
    SimpleWrapper() : val_{} {};
    explicit SimpleWrapper(const T &initial) : val_{initial} {};
    T &getRef() { return val_; }
    using value_type = T;

  private:
    T val_;
};

TEST_F(TApp, wrapperInt) {
    SimpleWrapper<int> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "2"};

    run();
    EXPECT_EQ(wrap.getRef(), 2);
}

TEST_F(TApp, wrapperString) {
    SimpleWrapper<std::string> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "str"};

    run();
    EXPECT_EQ(wrap.getRef(), "str");
}

TEST_F(TApp, wrapperVector) {
    SimpleWrapper<std::vector<int>> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "1", "2", "3", "4"};

    run();
    auto v1 = wrap.getRef();
    auto v2 = std::vector<int>{1, 2, 3, 4};
    EXPECT_EQ(v1, v2);
}

TEST_F(TApp, wrapperwrapperString) {
    SimpleWrapper<SimpleWrapper<std::string>> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "arg"};

    run();
    auto v1 = wrap.getRef().getRef();
    auto v2 = "arg";
    EXPECT_EQ(v1, v2);
}

TEST_F(TApp, wrapperwrapperVector) {
    SimpleWrapper<SimpleWrapper<std::vector<int>>> wrap;
    auto opt = app.add_option("--val", wrap);
    args = {"--val", "1", "2", "3", "4"};

    run();
    auto v1 = wrap.getRef().getRef();
    auto v2 = std::vector<int>{1, 2, 3, 4};
    EXPECT_EQ(v1, v2);
    opt->type_size(0, 5);

    args = {"--val"};

    run();
    EXPECT_TRUE(wrap.getRef().getRef().empty());

    args = {"--val", "happy", "sad"};

    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, wrapperComplex) {
    SimpleWrapper<std::complex<double>> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "1", "2"};

    run();
    auto &v1 = wrap.getRef();
    auto v2 = std::complex<double>{1, 2};
    EXPECT_EQ(v1.real(), v2.real());
    EXPECT_EQ(v1.imag(), v2.imag());
    args = {"--val", "1.4-4j"};

    run();
    v2 = std::complex<double>{1.4, -4};
    EXPECT_EQ(v1.real(), v2.real());
    EXPECT_EQ(v1.imag(), v2.imag());
}

TEST_F(TApp, vectorComplex) {
    std::vector<std::complex<double>> vcomplex;
    app.add_option("--val", vcomplex);
    args = {"--val", "1", "2", "--val", "1.4-4j"};

    run();

    ASSERT_EQ(vcomplex.size(), 2U);
    EXPECT_EQ(vcomplex[0].real(), 1.0);
    EXPECT_EQ(vcomplex[0].imag(), 2.0);
    EXPECT_EQ(vcomplex[1].real(), 1.4);
    EXPECT_EQ(vcomplex[1].imag(), -4.0);
}
