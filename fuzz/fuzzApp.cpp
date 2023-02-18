// Copyright (c) 2017-2023, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "fuzzApp.hpp"

namespace CLI {
/*
int32_t val32{0};
    int16_t val16{0};
    int8_t val8{0};
    int64_t val64{0};

    uint32_t uval32{0};
    uint16_t uval16{0};
    uint8_t uval8{0};
    uint64_t uval64{0};

    std::atomic<int64_t> atomicval64{0};
    std::atomic<uint64_t> atomicuval64{0};

    double v1{0};
    float v2{0};

    std::vector<double> vv1;
    std::vector<std::string> vstr;
    std::vector<std::vector<double>> vecvecd;
    std::vector<std::vector<std::string>> vvs;
    std::optional<double> od1;
    std::optional<std::string> ods;
    std::pair<double, std::string> p1;
    std::pair<std::vector<double>, std::string> p2;
    std::tuple<int64_t, uint16_t, std::optional<double>> t1;
    std::tuple<std::tuple<std::tuple<std::string, double, std::vector<int>>,std::string, double>,std::vector<int>,
std::optional<std::string>> tcomplex; std::string_view vstrv;

    bool flag1{false};
    int flagCnt{0};
    std::atomic<bool> flagAtomic{false};
    */
std::shared_ptr<CLI::App> FuzzApp::generateApp() {
    auto fApp = std::make_shared<CLI::App>("fuzzing App", "fuzzer");
    fApp->set_config("--config");
    fApp->add_flag("-a,--flag");
    fApp->add_flag("-b,--flag2", flag1);
    fApp->add_flag("-c{34},--flag3{1}", flagCnt)->disable_flag_override();
    fApp->add_flag("-e,--flagA", flagAtomic);

    fApp->add_option("-d,--opt1", val8);
    fApp->add_option("--opt2", val16);
    fApp->add_option("--opt3", val32);
    fApp->add_option("--opt4", val64);

    fApp->add_option("--opt5", uval8);
    fApp->add_option("--opt6", uval16);
    fApp->add_option("--opt7", uval32);
    fApp->add_option("--opt8", uval64);

    fApp->add_option("--aopt1", atomicval64);
    fApp->add_option("--aopt2", atomicuval64);

    fApp->add_option("--dopt1", v1);
    fApp->add_option("--dopt2", v2);

    fApp->add_option("--vopt1", vv1);
    fApp->add_option("--vopt2", vvs);
    fApp->add_option("--vopt3", vstr);
    fApp->add_option("--vopt4", vecvecd);

    fApp->add_option("--oopt1", od1);
    fApp->add_option("--oopt2", ods);

    fApp->add_option("--tup1", p1);
    fApp->add_option("--tup2", t1);
    fApp->add_option("--tup4", tcomplex);

    fApp->add_option("--dwrap", dwrap);
    fApp->add_option("--iwrap", iwrap);

    return fApp;
}

}  // namespace CLI
