// Copyright (c) 2017-2021, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"

#include <complex>
#include <cstdint>

using Catch::Matchers::Contains;

using cx = std::complex<double>;

TEST_CASE_METHOD(TApp, "ComplexOption", "[newparse]") {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp)->capture_default_str();

    args = {"-c", "4", "3"};

    std::string help = app.help();
    CHECK_THAT(help, Contains("1"));
    CHECK_THAT(help, Contains("2"));
    CHECK_THAT(help, Contains("COMPLEX"));

    CHECK(comp.real() == Approx(1));
    CHECK(comp.imag() == Approx(2));

    run();

    CHECK(comp.real() == Approx(4));
    CHECK(comp.imag() == Approx(3));
}

TEST_CASE_METHOD(TApp, "ComplexFloatOption", "[newparse]") {
    std::complex<float> comp{1, 2};
    app.add_option("-c,--complex", comp)->capture_default_str();

    args = {"-c", "4", "3"};

    std::string help = app.help();
    CHECK_THAT(help, Contains("1"));
    CHECK_THAT(help, Contains("2"));
    CHECK_THAT(help, Contains("COMPLEX"));

    CHECK(comp.real() == Approx(1));
    CHECK(comp.imag() == Approx(2));

    run();

    CHECK(comp.real() == Approx(4));
    CHECK(comp.imag() == Approx(3));
}

TEST_CASE_METHOD(TApp, "ComplexWithDelimiterOption", "[newparse]") {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp)->capture_default_str()->delimiter('+');

    args = {"-c", "4+3i"};

    std::string help = app.help();
    CHECK_THAT(help, Contains("1"));
    CHECK_THAT(help, Contains("2"));
    CHECK_THAT(help, Contains("COMPLEX"));

    CHECK(comp.real() == Approx(1));
    CHECK(comp.imag() == Approx(2));

    run();

    CHECK(comp.real() == Approx(4));
    CHECK(comp.imag() == Approx(3));

    args = {"-c", "5+-3i"};
    run();

    CHECK(comp.real() == Approx(5));
    CHECK(comp.imag() == Approx(-3));

    args = {"-c", "6", "-4i"};
    run();

    CHECK(comp.real() == Approx(6));
    CHECK(comp.imag() == Approx(-4));
}

TEST_CASE_METHOD(TApp, "ComplexIgnoreIOption", "[newparse]") {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp);

    args = {"-c", "4", "3i"};

    run();

    CHECK(comp.real() == Approx(4));
    CHECK(comp.imag() == Approx(3));
}

TEST_CASE_METHOD(TApp, "ComplexSingleArgOption", "[newparse]") {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp);

    args = {"-c", "4"};
    run();
    CHECK(comp.real() == Approx(4));
    CHECK(comp.imag() == Approx(0));

    args = {"-c", "4-2i"};
    run();
    CHECK(comp.real() == Approx(4));
    CHECK(comp.imag() == Approx(-2));
    args = {"-c", "4+2i"};
    run();
    CHECK(comp.real() == Approx(4));
    CHECK(comp.imag() == Approx(2));

    args = {"-c", "-4+2j"};
    run();
    CHECK(comp.real() == Approx(-4));
    CHECK(comp.imag() == Approx(2));

    args = {"-c", "-4.2-2j"};
    run();
    CHECK(comp.real() == Approx(-4.2));
    CHECK(comp.imag() == Approx(-2));

    args = {"-c", "-4.2-2.7i"};
    run();
    CHECK(comp.real() == Approx(-4.2));
    CHECK(comp.imag() == Approx(-2.7));
}

TEST_CASE_METHOD(TApp, "ComplexSingleImagOption", "[newparse]") {
    cx comp{1, 2};
    app.add_option("-c,--complex", comp);

    args = {"-c", "4j"};
    run();
    CHECK(comp.real() == Approx(0));
    CHECK(comp.imag() == Approx(4));

    args = {"-c", "-4j"};
    run();
    CHECK(comp.real() == Approx(0));
    CHECK(comp.imag() == Approx(-4));
    args = {"-c", "-4"};
    run();
    CHECK(comp.real() == Approx(-4));
    CHECK(comp.imag() == Approx(0));
    args = {"-c", "+4"};
    run();
    CHECK(comp.real() == Approx(4));
    CHECK(comp.imag() == Approx(0));
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

TEST_CASE_METHOD(TApp, "custom_string_converter", "[newparse]") {
    spair val;
    app.add_option("-d,--dual_string", val);

    args = {"-d", "string1:string2"};

    run();
    CHECK("string1" == val.first);
    CHECK("string2" == val.second);
}

TEST_CASE_METHOD(TApp, "custom_string_converterFail", "[newparse]") {
    spair val;
    app.add_option("-d,--dual_string", val);

    args = {"-d", "string2"};

    CHECK_THROWS_AS(run(), CLI::ConversionError);
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
TEST_CASE_METHOD(TApp, "stringWrapper", "[newparse]") {
    objWrapper<std::string> sWrapper;
    app.add_option("-v", sWrapper);
    args = {"-v", "string test"};

    run();

    CHECK("string test" == sWrapper.value());
}

static_assert(CLI::detail::is_direct_constructible<objWrapper<double>, double>::value,
              "double wrapper isn't properly assignable");

static_assert(!CLI::detail::is_direct_constructible<objWrapper<double>, int>::value,
              "double wrapper can be assigned from int");

static_assert(!CLI::detail::is_istreamable<objWrapper<double>>::value,
              "double wrapper is input streamable and it shouldn't be");

TEST_CASE_METHOD(TApp, "doubleWrapper", "[newparse]") {
    objWrapper<double> dWrapper;
    app.add_option("-v", dWrapper);
    args = {"-v", "2.36"};

    run();

    CHECK(2.36 == dWrapper.value());

    args = {"-v", "thing"};

    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

static_assert(CLI::detail::is_direct_constructible<objWrapper<int>, int>::value,
              "int wrapper is not constructible from int64");

static_assert(!CLI::detail::is_direct_constructible<objWrapper<int>, double>::value,
              "int wrapper is constructible from double");

static_assert(!CLI::detail::is_istreamable<objWrapper<int>>::value,
              "int wrapper is input streamable and it shouldn't be");

TEST_CASE_METHOD(TApp, "intWrapper", "[newparse]") {
    objWrapper<int> iWrapper;
    app.add_option("-v", iWrapper);
    args = {"-v", "45"};

    run();

    CHECK(45 == iWrapper.value());
    args = {"-v", "thing"};

    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

static_assert(!CLI::detail::is_direct_constructible<objWrapper<float>, int>::value,
              "float wrapper is constructible from int");
static_assert(!CLI::detail::is_direct_constructible<objWrapper<float>, double>::value,
              "float wrapper is constructible from double");

static_assert(!CLI::detail::is_istreamable<objWrapper<float>>::value,
              "float wrapper is input streamable and it shouldn't be");

TEST_CASE_METHOD(TApp, "floatWrapper", "[newparse]") {
    objWrapper<float> iWrapper;
    app.add_option<objWrapper<float>, float>("-v", iWrapper);
    args = {"-v", "45.3"};

    run();

    CHECK(45.3f == iWrapper.value());
    args = {"-v", "thing"};

    CHECK_THROWS_AS(run(), CLI::ConversionError);
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

TEST_CASE_METHOD(TApp, "dobjWrapper", "[newparse]") {
    dobjWrapper iWrapper;
    app.add_option("-v", iWrapper);
    args = {"-v", "45"};

    run();

    CHECK(45 == iWrapper.ivalue());
    CHECK(0.0 == iWrapper.dvalue());

    args = {"-v", "thing"};

    CHECK_THROWS_AS(run(), CLI::ConversionError);
    iWrapper = dobjWrapper{};

    args = {"-v", "45.1"};

    run();
    CHECK(0 == iWrapper.ivalue());
    CHECK(45.1 == iWrapper.dvalue());
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

TEST_CASE_METHOD(TApp, "uint16Wrapper", "[newparse]") {
    AobjWrapper<std::uint16_t> sWrapper;
    app.add_option<AobjWrapper<std::uint16_t>, std::uint16_t>("-v", sWrapper);
    args = {"-v", "9"};

    run();

    CHECK(9u == sWrapper.value());
    args = {"-v", "thing"};

    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-v", "72456245754"};

    CHECK_THROWS_AS(run(), CLI::ConversionError);

    args = {"-v", "-3"};

    CHECK_THROWS_AS(run(), CLI::ConversionError);
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

TEST_CASE_METHOD(TApp, "wrapperInt", "[newparse]") {
    SimpleWrapper<int> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "2"};

    run();
    CHECK(2 == wrap.getRef());
}

TEST_CASE_METHOD(TApp, "wrapperString", "[newparse]") {
    SimpleWrapper<std::string> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "str"};

    run();
    CHECK("str" == wrap.getRef());
}

TEST_CASE_METHOD(TApp, "wrapperVector", "[newparse]") {
    SimpleWrapper<std::vector<int>> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "1", "2", "3", "4"};

    run();
    auto v1 = wrap.getRef();
    auto v2 = std::vector<int>{1, 2, 3, 4};
    CHECK(v2 == v1);
}

TEST_CASE_METHOD(TApp, "wrapperwrapperString", "[newparse]") {
    SimpleWrapper<SimpleWrapper<std::string>> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "arg"};

    run();
    auto v1 = wrap.getRef().getRef();
    auto v2 = "arg";
    CHECK(v2 == v1);
}

TEST_CASE_METHOD(TApp, "wrapperwrapperVector", "[newparse]") {
    SimpleWrapper<SimpleWrapper<std::vector<int>>> wrap;
    auto opt = app.add_option("--val", wrap);
    args = {"--val", "1", "2", "3", "4"};

    run();
    auto v1 = wrap.getRef().getRef();
    auto v2 = std::vector<int>{1, 2, 3, 4};
    CHECK(v2 == v1);
    opt->type_size(0, 5);

    args = {"--val"};

    run();
    CHECK(wrap.getRef().getRef().empty());

    args = {"--val", "happy", "sad"};

    CHECK_THROWS_AS(run(), CLI::ConversionError);
}

TEST_CASE_METHOD(TApp, "wrapperComplex", "[newparse]") {
    SimpleWrapper<std::complex<double>> wrap;
    app.add_option("--val", wrap);
    args = {"--val", "1", "2"};

    run();
    auto &v1 = wrap.getRef();
    auto v2 = std::complex<double>{1, 2};
    CHECK(v2.real() == v1.real());
    CHECK(v2.imag() == v1.imag());
    args = {"--val", "1.4-4j"};

    run();
    v2 = std::complex<double>{1.4, -4};
    CHECK(v2.real() == v1.real());
    CHECK(v2.imag() == v1.imag());
}

TEST_CASE_METHOD(TApp, "vectorComplex", "[newparse]") {
    std::vector<std::complex<double>> vcomplex;
    app.add_option("--val", vcomplex);
    args = {"--val", "1", "2", "--val", "1.4-4j"};

    run();

    REQUIRE(2U == vcomplex.size());
    CHECK(1.0 == vcomplex[0].real());
    CHECK(2.0 == vcomplex[0].imag());
    CHECK(1.4 == vcomplex[1].real());
    CHECK(-4.0 == vcomplex[1].imag());
}
