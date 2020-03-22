// Copyright (c) 2017-2020, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "app_helper.hpp"
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/slist.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/stable_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/container/vector.hpp>
#include <string>
#include <vector>

#include "gmock/gmock.h"

namespace boost {
namespace container {

template <class T> class TApp_container_single_boost : public TApp {
  public:
    using container_type = T;
    container_type cval{};
    TApp_container_single_boost() : TApp() {}
};

using containerTypes_single_boost =
    ::testing::Types<small_vector<int, 2>, small_vector<int, 3>, flat_set<int>, stable_vector<int>, slist<int>>;

TYPED_TEST_SUITE(TApp_container_single_boost, containerTypes_single_boost, );

TYPED_TEST(TApp_container_single_boost, containerInt_boost) {

    auto &cv = TApp_container_single_boost<TypeParam>::cval;
    CLI::Option *opt = (TApp::app).add_option("-v", cv);

    TApp::args = {"-v", "1", "-1", "-v", "3", "-v", "-976"};
    TApp::run();
    EXPECT_EQ(4u, (TApp::app).count("-v"));
    EXPECT_EQ(4u, cv.size());
    opt->check(CLI::PositiveNumber.application_index(0));
    opt->check((!CLI::PositiveNumber).application_index(1));
    EXPECT_NO_THROW(TApp::run());
    EXPECT_EQ(4u, cv.size());
    // v[3] would be negative
    opt->check(CLI::PositiveNumber.application_index(3));
    EXPECT_THROW(TApp::run(), CLI::ValidationError);
}

template <class T> class TApp_container_pair_boost : public TApp {
  public:
    using container_type = T;
    container_type cval{};
    TApp_container_pair_boost() : TApp() {}
};

using isp = std::pair<int, std::string>;
using containerTypes_pair_boost = ::testing::
    Types<stable_vector<isp>, small_vector<isp, 2>, flat_set<isp>, slist<isp>, vector<isp>, flat_map<int, std::string>>;

TYPED_TEST_SUITE(TApp_container_pair_boost, containerTypes_pair_boost, );

TYPED_TEST(TApp_container_pair_boost, containerPair_boost) {

    auto &cv = TApp_container_pair_boost<TypeParam>::cval;
    (TApp::app).add_option("--dict", cv);

    TApp::args = {"--dict", "1", "str1", "--dict", "3", "str3"};

    TApp::run();
    EXPECT_EQ(cv.size(), 2u);

    TApp::args = {"--dict", "1", "str1", "--dict", "3", "--dict", "-1", "str4"};
    TApp::run();
    EXPECT_EQ(cv.size(), 3u);
}

template <class T> class TApp_container_tuple_boost : public TApp {
  public:
    using container_type = T;
    container_type cval{};
    TApp_container_tuple_boost() : TApp() {}
};

using tup_obj = std::tuple<int, std::string, double>;
using containerTypes_tuple_boost =
    ::testing::Types<small_vector<tup_obj, 3>, stable_vector<tup_obj>, flat_set<tup_obj>, slist<tup_obj>>;

TYPED_TEST_SUITE(TApp_container_tuple_boost, containerTypes_tuple_boost, );

TYPED_TEST(TApp_container_tuple_boost, containerTuple_boost) {

    auto &cv = TApp_container_tuple_boost<TypeParam>::cval;
    (TApp::app).add_option("--dict", cv);

    TApp::args = {"--dict", "1", "str1", "4.3", "--dict", "3", "str3", "2.7"};

    TApp::run();
    EXPECT_EQ(cv.size(), 2u);

    TApp::args = {"--dict", "1", "str1", "4.3", "--dict", "3", "str3", "2.7", "--dict", "-1", "str4", "-1.87"};
    TApp::run();
    EXPECT_EQ(cv.size(), 3u);
}

using icontainer1 = vector<int>;
using icontainer2 = flat_set<int>;
using icontainer3 = slist<int>;
using containerTypes_container_boost = ::testing::Types<std::vector<icontainer1>,
                                                        slist<icontainer1>,
                                                        flat_set<icontainer1>,
                                                        small_vector<icontainer1, 2>,
                                                        std::vector<icontainer2>,
                                                        slist<icontainer2>,
                                                        flat_set<icontainer2>,
                                                        stable_vector<icontainer2>,
                                                        static_vector<icontainer3, 10>,
                                                        slist<icontainer3>,
                                                        flat_set<icontainer3>,
                                                        static_vector<icontainer3, 10>>;

template <class T> class TApp_container_container_boost : public TApp {
  public:
    using container_type = T;
    container_type cval{};
    TApp_container_container_boost() : TApp() {}
};

TYPED_TEST_SUITE(TApp_container_container_boost, containerTypes_container_boost, );

TYPED_TEST(TApp_container_container_boost, containerContainer_boost) {

    auto &cv = TApp_container_container_boost<TypeParam>::cval;
    (TApp::app).add_option("--dict", cv);

    TApp::args = {"--dict", "1", "2", "4", "--dict", "3", "1"};

    TApp::run();
    EXPECT_EQ(cv.size(), 2u);

    TApp::args = {"--dict", "1", "2", "4", "--dict", "3", "1", "--dict", "3", "--dict",
                  "3",      "3", "3", "3", "3",      "3", "3", "3",      "3", "-3"};
    TApp::run();
    EXPECT_EQ(cv.size(), 4u);
}

}  // namespace container
}  // namespace boost
