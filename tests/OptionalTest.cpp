#include <cstdlib>
#include <iostream>

#ifdef __has_include
#if __has_include(<optional>)
#include <optional>
#define have_optional 1
using std::experimental::optional;
#elif __has_include(<experimental/optional>)
#include <experimental/optional>
#define have_optional 1
using std::optional;
#else
#define have_optional 0
#endif
#endif

#if have_optional

template <typename T> std::istream &operator>>(std::istream &in, optional<T> &val) {
    T v;
    in >> v;
    val = v;
    return in;
}

#include "app_helper.hpp"

TEST_F(TApp, OptionalTest) {
    optional<int> opt;
    app.add_option("-c,--count", opt);
    run();
    EXPECT_FALSE(opt);

    app.reset();
    args = {"-c", "1"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 1);

    app.reset();
    args = {"--count", "3"};
    run();
    EXPECT_TRUE(opt);
    EXPECT_EQ(*opt, 3);
}

#endif
