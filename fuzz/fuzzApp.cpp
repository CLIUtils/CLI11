// Copyright (c) 2017-2024, University of Cincinnati, developed by Henry Schreiner
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
    fApp->add_flag("-b,--flag2,!--nflag2", flag1);
    fApp->add_flag("-c{34},--flag3{1}", flagCnt)->disable_flag_override();
    fApp->add_flag("-e,--flagA", flagAtomic);
    fApp->add_flag("--atd", doubleAtomic);

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

    auto *vgroup = fApp->add_option_group("vectors");

    vgroup->add_option("--vopt1", vv1);
    vgroup->add_option("--vopt2", vvs)->inject_separator();
    vgroup->add_option("--vopt3", vstr);
    vgroup->add_option("--vopt4", vecvecd)->inject_separator();

    fApp->add_option("--oopt1", od1);
    fApp->add_option("--oopt2", ods);

    fApp->add_option("--tup1", p1);
    fApp->add_option("--tup2", t1);
    fApp->add_option("--tup4", tcomplex);
    vgroup->add_option("--vtup", vectup);

    fApp->add_option("--dwrap", dwrap);
    fApp->add_option("--iwrap", iwrap);
    fApp->add_option("--swrap", swrap);
    // file checks
    fApp->add_option("--dexists")->check(ExistingDirectory);
    fApp->add_option("--fexists")->check(ExistingFile);
    fApp->add_option("--fnexists")->check(NonexistentPath);

    auto *sub = fApp->add_subcommand("sub1");

    sub->add_option("--sopt2", val16)->check(Range(1, 10));
    sub->add_option("--sopt3", val32)->check(PositiveNumber);
    sub->add_option("--sopt4", val64)->check(NonNegativeNumber);

    sub->add_option("--sopt5", uval8)->transform(Bound(6, 20));
    sub->add_option("--sopt6", uval16);
    sub->add_option("--sopt7", uval32);
    sub->add_option("--sopt8", uval64);

    sub->add_option("--saopt1", atomicval64);
    sub->add_option("--saopt2", atomicuval64);

    sub->add_option("--sdopt1", v1);
    sub->add_option("--sdopt2", v2);

    sub->add_option("--svopt1", vv1);
    sub->add_option("--svopt2", vvs);
    sub->add_option("--svopt3", vstr);
    sub->add_option("--svopt4", vecvecd);

    sub->add_option("--soopt1", od1);
    sub->add_option("--soopt2", ods);

    sub->add_option("--stup1", p1);
    sub->add_option("--stup2", t1);
    sub->add_option("--stup4", tcomplex);
    sub->add_option("--svtup", vectup);

    sub->add_option("--sdwrap", dwrap);
    sub->add_option("--siwrap", iwrap);

    auto *resgroup = fApp->add_option_group("outputOrder");

    resgroup->add_option("--vA", vstrA)->expected(0, 2)->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
    resgroup->add_option("--vB", vstrB)->expected(0, 2)->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
    resgroup->add_option("--vC", vstrC)->expected(0, 2)->multi_option_policy(CLI::MultiOptionPolicy::TakeFirst);
    resgroup->add_option("--vD", vstrD)->expected(0, 2)->multi_option_policy(CLI::MultiOptionPolicy::Reverse);
    resgroup->add_option("--vS", val32)->expected(0, 2)->multi_option_policy(CLI::MultiOptionPolicy::Sum);
    resgroup->add_option("--vM", mergeBuffer)->expected(0, 2)->multi_option_policy(CLI::MultiOptionPolicy::Join);
    resgroup->add_option("--vE", vstrE)->expected(2, 4)->delimiter(',');

    auto *vldtr = fApp->add_option_group("validators");

    validator_strings.resize(10);
    vldtr->add_option("--vdtr1", validator_strings[0])->join()->check(CLI::PositiveNumber);
    vldtr->add_option("--vdtr2", validator_strings[1])->join()->check(CLI::NonNegativeNumber);
    vldtr->add_option("--vdtr3", validator_strings[2])->join()->check(CLI::NonexistentPath);
    vldtr->add_option("--vdtr4", validator_strings[3])->join()->check(CLI::Range(7, 3456));
    vldtr->add_option("--vdtr5", validator_strings[4])
        ->join()
        ->check(CLI::Range(std::string("aa"), std::string("zz"), "string range"));
    vldtr->add_option("--vdtr6", validator_strings[5])->join()->check(CLI::TypeValidator<double>());
    vldtr->add_option("--vdtr7", validator_strings[6])->join()->check(CLI::TypeValidator<bool>());
    vldtr->add_option("--vdtr8", validator_strings[7])->join()->check(CLI::ValidIPV4);
    vldtr->add_option("--vdtr9", validator_strings[8])->join()->transform(CLI::Bound(2, 255));
    return fApp;
}

}  // namespace CLI
