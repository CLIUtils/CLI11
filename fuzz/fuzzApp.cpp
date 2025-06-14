// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include "fuzzApp.hpp"
#include <algorithm>
#include <iostream>

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
    fApp->set_help_all_flag("--help-all");
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

    fApp->add_option("--cv3", cv3);
    fApp->add_option("--cv4", cv4);

    auto *vgroup = fApp->add_option_group("vectors");

    vgroup->add_option("--vopt1", vv1);
    vgroup->add_option("--vopt2", vvs)->inject_separator();
    vgroup->add_option("--vopt3", vstr);
    vgroup->add_option("--vopt4", vecvecd)->inject_separator();

    fApp->add_option("--oopt1", od1);
    fApp->add_option("--oopt2", ods);
    fApp->add_option("--ovopt", ovs);

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
    sub->add_option("--stup4", tcomplex2);
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

static void print_string_comparison(const std::string &s1,
                                    const std::string &s2,
                                    const std::string &prefix,
                                    const std::string &s1name,
                                    const std::string &s2name) {
    for(size_t jj = 0; jj < (std::max)(s1.size(), s2.size()); ++jj) {
        if(jj >= s1.size()) {
            std::cout << prefix << ":" << s1name << "[" << jj << "] = [empty], " << s2name << "[" << jj
                      << "]=" << static_cast<int>(s2[jj]) << '\n';
        } else if(jj >= s2.size()) {
            std::cout << prefix << ":" << s1name << "[" << jj << "]=" << static_cast<int>(s1[jj]) << ", " << s2name
                      << "[" << jj << "]=[empty] \n";
        } else if(s1[jj] != s2[jj]) {
            std::cout << "-->" << prefix << ":" << s1name << "[" << jj << "]=" << static_cast<int>(s1[jj]) << ", "
                      << s2name << "[" << jj << "]=" << static_cast<int>(s2[jj]) << '\n';
        } else {
            std::cout << prefix << ":" << s1name << "[" << jj << "]=" << static_cast<int>(s1[jj]) << '\n';
        }
    }
}

static constexpr double keydub = 6262542.2622;
bool FuzzApp::compare(const FuzzApp &other, bool print_error) const {
    if(val32 != other.val32) {
        return false;
    }
    if(val16 != other.val16) {
        return false;
    }
    if(val8 != other.val8) {
        return false;
    }
    if(val64 != other.val64) {
        return false;
    }

    if(uval32 != other.uval32) {
        return false;
    }
    if(uval16 != other.uval16) {
        return false;
    }
    if(uval8 != other.uval8) {
        return false;
    }
    if(uval64 != other.uval64) {
        return false;
    }

    if(atomicval64 != other.atomicval64) {
        return false;
    }
    if(atomicuval64 != other.atomicuval64) {
        return false;
    }

    if(v1 != other.v1) {
        if(!(std::isnan(v1) && std::isnan(other.v1))) {
            return false;
        }
    }
    if(v2 != other.v2) {
        if(!(std::isnan(v2) && std::isnan(other.v2))) {
            return false;
        }
    }

    if(vv1 != other.vv1) {
        if(vv1.size() != other.vv1.size()) {
            return false;
        }
        // need to check if they are both nan
        for(std::size_t index = 0; index < vv1.size(); ++index) {
            if(vv1[index] != other.vv1[index]) {
                if(std::isnan(vv1[index]) && std::isnan(other.vv1[index])) {
                    continue;
                }
                return false;
            }
        }
    }
    if(vstr != other.vstr) {
        return false;
    }

    if(vecvecd != other.vecvecd) {
        if(vecvecd.size() != other.vecvecd.size()) {
            return false;
        }
        // need to check if they are both nan
        for(std::size_t index = 0; index < vecvecd.size(); ++index) {
            if(vecvecd[index].size() != other.vecvecd[index].size()) {
                return false;
            }
            if(vecvecd[index] != other.vecvecd[index]) {
                for(std::size_t jj = 0; jj < vecvecd[index].size(); ++jj) {
                    if(std::isnan(vecvecd[index][jj]) && std::isnan(other.vecvecd[index][jj])) {
                        continue;
                    }
                    return false;
                }
            }
        }
    }
    if(vvs != other.vvs) {
        return false;
    }
    if(od1 != other.od1) {
        if(!od1 || !other.od1) {
            return false;
        }
        if(!(std::isnan(*od1) && std::isnan(*other.od1))) {
            return false;
        }
    }
    if(ods != other.ods) {
        return false;
    }
    if(ovs != other.ovs) {
        return false;
    }
    if(p1 != other.p1) {
        if(p1.second != other.p1.second) {
            return false;
        }
        if(!(std::isnan(p1.first) && std::isnan(other.p1.first))) {
            return false;
        }
    }
    if(p2 != other.p2) {
        if(p2.second != other.p2.second) {
            return false;
        }
        if(p2.first.size() != other.p2.first.size()) {
            return false;
        }
        for(std::size_t index = 0; index < p2.first.size(); ++index) {
            if(p2.first[index] != other.p2.first[index]) {
                if(std::isnan(p2.first[index]) && std::isnan(other.p2.first[index])) {
                    continue;
                }
                return false;
            }
        }
    }
    if(t1 != other.t1) {
        return false;
    }
    if(cv3 != other.cv3) {
        if(cv3.real() != other.cv3.real()) {
            if(!(std::isnan(cv3.real()) && std::isnan(other.cv3.real()))) {
                return false;
            }
        }
        if(cv3.imag() != other.cv3.imag()) {
            if(!(std::isnan(cv3.imag()) && std::isnan(other.cv3.imag()))) {
                return false;
            }
        }
    }
    if(cv4 != other.cv4) {
        if(cv4.real() != other.cv4.real()) {
            if(!(std::isnan(cv4.real()) && std::isnan(other.cv4.real()))) {
                return false;
            }
        }
        if(cv4.imag() != other.cv4.imag()) {
            if(!(std::isnan(cv4.imag()) && std::isnan(other.cv4.imag()))) {
                return false;
            }
        }
    }
    if(tcomplex != other.tcomplex) {

        if(std::get<0>(tcomplex) != std::get<0>(other.tcomplex)) {
            auto testa = std::get<0>(tcomplex);
            auto testb = std::get<0>(other.tcomplex);
            if(std::isnan(std::get<double>(testa))) {
                std::get<double>(testa) = keydub;
            }
            if(std::isnan(std::get<double>(testb))) {
                std::get<double>(testb) = keydub;
            }
            if(std::isnan(std::get<double>(std::get<0>(testa)))) {
                std::get<double>(std::get<0>(testa)) = keydub;
            }
            if(std::isnan(std::get<double>(std::get<0>(testb)))) {
                std::get<double>(std::get<0>(testb)) = keydub;
            }
            if(testa != testb) {
                return false;
            }

        } else {
            return false;
        }
    }
    if(tcomplex2 != other.tcomplex2) {
        if(std::get<0>(tcomplex2) != std::get<0>(other.tcomplex2)) {
            auto testa = std::get<0>(tcomplex2);
            auto testb = std::get<0>(other.tcomplex2);
            if(std::isnan(std::get<double>(testa))) {
                std::get<double>(testa) = keydub;
            }
            if(std::isnan(std::get<double>(testb))) {
                std::get<double>(testb) = keydub;
            }
            if(std::isnan(std::get<double>(std::get<0>(testa)))) {
                std::get<double>(std::get<0>(testa)) = keydub;
            }
            if(std::isnan(std::get<double>(std::get<0>(testb)))) {
                std::get<double>(std::get<0>(testb)) = keydub;
            }
            if(testa != testb) {
                return false;
            }
        } else {
            return false;
        }
    }
    if(vectup != other.vectup) {
        bool found_diff = false;
        if(vectup.size() != other.vectup.size()) {
            if(print_error) {
                std::cout << "size is different vectup.size()=" << vectup.size()
                          << " other.vectup.size=" << other.vectup.size() << '\n';
            }
            found_diff = true;
        } else {
            for(size_t ii = 0; ii < vectup.size(); ++ii) {
                if(vectup[ii] != other.vectup[ii]) {
                    int matching = 4;
                    if(std::get<0>(vectup[ii]) != std::get<0>(other.vectup[ii])) {
                        --matching;
                        if(print_error) {
                            std::cout << "vectup[" << ii << "][0] != other.vectup[" << ii << "][0]\n";
                        }
                    }
                    if(std::get<1>(vectup[ii]) != std::get<1>(other.vectup[ii])) {
                        if(!(std::isnan(std::get<1>(vectup[ii])) && std::isnan(std::get<1>(other.vectup[ii])))) {
                            --matching;
                            if(print_error) {
                                std::cout << "vectup[" << ii << "][1] != other.vectup[" << ii << "][1]\n";
                            }
                        }
                    }
                    if(std::get<2>(vectup[ii]) != std::get<2>(other.vectup[ii])) {
                        --matching;
                        if(print_error) {
                            std::cout << "vectup[" << ii << "][2] != other.vectup[" << ii << "][2]\n";
                        }
                    }
                    if(std::get<3>(vectup[ii]) != std::get<3>(other.vectup[ii])) {
                        --matching;
                        if(print_error) {
                            std::cout << "vectup[" << ii << "][3] != other.vectup[" << ii << "][3]\n";
                        }
                    }
                    if(matching != 4) {
                        found_diff = true;
                        if(print_error) {
                            std::cout << "vectup[" << ii << "] != other.vectup[" << ii << "]\n";
                        }
                    }
                }
                /*print_string_comparison(vstrA[ii],
                    other.vectup[ii],
                    std::string("string[") + std::to_string(ii) + ']',
                    "vstrA",
                    "other.vstrA");
                    */
            }
        }
        if(found_diff) {
            return false;
        }
    }
    if(vstrv != other.vstrv) {
        return false;
    }

    if(flag1 != other.flag1) {
        return false;
    }
    if(flagCnt != other.flagCnt) {
        return false;
    }
    if(flagAtomic != other.flagAtomic) {
        return false;
    }

    if(iwrap.value() != other.iwrap.value()) {
        return false;
    }
    if(dwrap.value() != other.dwrap.value()) {
        if(!(std::isnan(dwrap.value()) && std::isnan(other.dwrap.value()))) {
            return false;
        }
    }
    if(swrap.value() != other.swrap.value()) {
        return false;
    }
    if(buffer != other.buffer) {
        return false;
    }
    if(intbuffer != other.intbuffer) {
        return false;
    }
    if(doubleAtomic != other.doubleAtomic) {
        if(!(std::isnan(doubleAtomic.load()) && std::isnan(other.doubleAtomic.load()))) {
            return false;
        }
    }

    // for testing restrictions and reduction methods
    if(vstrA != other.vstrA) {
        if(print_error) {
            if(vstrA.size() != other.vstrA.size()) {
                std::cout << "size is different vstrA.size()=" << vstrA.size()
                          << " other.vstrA.size=" << other.vstrA.size() << '\n';
            } else {
                for(size_t ii = 0; ii < vstrA.size(); ++ii) {
                    print_string_comparison(vstrA[ii],
                                            other.vstrA[ii],
                                            std::string("string[") + std::to_string(ii) + ']',
                                            "vstrA",
                                            "other.vstrA");
                }
            }
        }
        return false;
    }
    if(vstrB != other.vstrB) {
        return false;
    }
    if(vstrC != other.vstrC) {
        return false;
    }
    if(vstrD != other.vstrD) {
        // the return result if reversed so it can alternate
        std::vector<std::string> res = vstrD;
        std::reverse(res.begin(), res.end());
        if(res != other.vstrD) {
            if(print_error) {
                if(res.size() != other.vstrD.size()) {
                    std::cout << "size is different vstrD.size()=" << res.size()
                              << " other.vstrD.size=" << other.vstrD.size() << '\n';
                } else {
                    for(size_t ii = 0; ii < res.size(); ++ii) {
                        print_string_comparison(res[ii],
                                                other.vstrD[ii],
                                                std::string("string[") + std::to_string(ii) + ']',
                                                "vstrD",
                                                "other.vstrD");
                    }
                }
            }
            return false;
        }
    }
    if(vstrE != other.vstrE) {
        return false;
    }
    if(vstrF != other.vstrF) {
        return false;
    }
    if(mergeBuffer != other.mergeBuffer) {
        return false;
    }
    if(validator_strings != other.validator_strings) {
        return false;
    }
    // now test custom string_options
    if(custom_string_options.size() != other.custom_string_options.size()) {
        return false;
    }
    for(std::size_t ii = 0; ii < custom_string_options.size(); ++ii) {
        if(custom_string_options[ii]->first != other.custom_string_options[ii]->first) {
            if(custom_string_options[ii]->second) {
                if(print_error) {
                    print_string_comparison(custom_string_options[ii]->first,
                                            other.custom_string_options[ii]->first,
                                            std::string("custom_string[") + std::to_string(ii) + ']',
                                            "c1",
                                            "other.c1");
                }
                return false;
            }
        }
    }
    // now test custom vector_options
    if(custom_vector_options.size() != other.custom_vector_options.size()) {
        return false;
    }
    for(std::size_t ii = 0; ii < custom_vector_options.size(); ++ii) {
        if(custom_vector_options[ii]->first != other.custom_vector_options[ii]->first) {
            if(custom_vector_options[ii]->second) {
                return false;
            }
        }
    }
    return true;
}

void FuzzApp::modify_option(CLI::Option *opt, const std::string &modifier_string) {
    auto mod_start = modifier_string.find("modifiers=");
    if(mod_start == std::string::npos) {
        return;
    }
    auto mod_end = modifier_string.find_first_of(' ', mod_start + 1);
    std::string modifiers = modifier_string.substr(mod_start + 10, mod_end - mod_start - 10);
    for(const auto mod : modifiers) {
        switch(mod) {
        case 'r':
        case 'R':
            opt->required(mod < '`');
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
            opt->expected(mod - '0');
            break;
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            opt->expected(opt->get_expected_min(), mod - '5');
            break;
        case 'c':
        case 'C':
            opt->ignore_case(mod < '`');
            break;
        case 'u':
        case 'U':
            opt->ignore_underscore(mod < '`');
            break;
        case 'f':
        case 'F':
            opt->disable_flag_override(mod < '`');
            break;
        case 'e':
        case 'E':
            opt->allow_extra_args(mod < '`');
            break;
        case ',':
        case ';':
        case '%':
        case '#':
        case '|':
        case '\\':
        case '~':
            opt->delimiter(mod);
            break;
        case 'g':
        case 'G':
            opt->configurable(mod < '`');
            break;
        case 'p':
        case 'P':
            opt->trigger_on_parse(mod < '`');
            break;
        case 't':
        case 'T':
            opt->multi_option_policy(CLI::MultiOptionPolicy::Throw);
            break;
        case 'l':
        case 'L':
            opt->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
            break;
        case 's':
        case 'S':
            opt->multi_option_policy(CLI::MultiOptionPolicy::TakeFirst);
            break;
        case 'a':
        case 'A':
            opt->multi_option_policy(CLI::MultiOptionPolicy::TakeAll);
            break;
        case 'v':
        case 'V':
            opt->multi_option_policy(CLI::MultiOptionPolicy::Reverse);
            break;
        case 'j':
        case 'J':
            opt->multi_option_policy(CLI::MultiOptionPolicy::Join);
            break;
        case '+':
            opt->multi_option_policy(CLI::MultiOptionPolicy::Sum);
            break;
        case 'i':
            opt->check(CLI::Number);
            break;
        case 'I':
            opt->check(CLI::NonNegativeNumber);
            break;
        case 'w':
            opt->check(!CLI::Number);
            break;
        default:
            break;
        }
    }
}

void FuzzApp::modify_subcommand(CLI::App *app, const std::string &modifiers) {
    for(const auto mod : modifiers) {
        switch(mod) {
        case 'w':
        case 'W':
            app->allow_windows_style_options(mod < '`');
            break;
        case 'n':
        case 'N':
            app->allow_non_standard_option_names(mod < '`');
            break;
        case 'p':
        case 'P':
            app->allow_subcommand_prefix_matching(mod < '`');
            break;
        case 'f':
        case 'F':
            app->fallthrough(mod < '`');
            break;
        case 'v':
        case 'V':
            app->validate_positionals(mod < '`');
            break;
        case 'e':
        case 'E':
            app->positionals_at_end(mod < '`');
            break;
        default:
            break;
        }
    }
}

SubcommandData extract_subcomand_info(const std::string &description_string, std::size_t index) {
    SubcommandData sub_data;
    sub_data.next = index;
    int depth = 1;
    // end of prefix section for <subcommand
    auto first_sub_label = description_string.find_first_of('>', index + 12);
    if(first_sub_label == std::string::npos) {
        return sub_data;
    }
    auto end_sub_label = first_sub_label;
    auto end_sub = description_string.find("</subcommand>", end_sub_label + 1);
    auto start_sub = description_string.find("<subcommand", end_sub_label + 1);
    while(depth > 0) {
        if(end_sub == std::string::npos) {
            return sub_data;
        }
        depth += (end_sub < start_sub) ? -1 : 1;

        if(depth > 0) {
            if(start_sub != std::string::npos) {
                end_sub_label = description_string.find_first_of('>', start_sub + 12);
                if(end_sub_label == std::string::npos) {
                    return sub_data;
                }
                end_sub = description_string.find("</subcommand>", end_sub_label + 1);
                start_sub = description_string.find("<subcommand", end_sub_label + 1);
            } else {
                end_sub = description_string.find("</subcommand>", end_sub + 12);
            }
        }
    }
    sub_data.data = description_string.substr(first_sub_label + 1, end_sub - first_sub_label - 1);
    std::string metadata = description_string.substr(index + 12, end_sub_label - index - 12);
    auto fields = detail::split_up(metadata);
    for(auto &field : fields) {
        if(field.compare(0, 5, "name=") == 0) {
            sub_data.name = field.substr(5);
            detail::process_quoted_string(sub_data.name);
        } else if(field.compare(0, 11, "description=") == 0) {
            sub_data.description = field.substr(11);
            detail::process_quoted_string(sub_data.description);
        } else if(field.compare(0, 10, "modifiers=") == 0) {
            sub_data.modifiers = field.substr(10);
            detail::process_quoted_string(sub_data.modifiers);
        }
    }
    sub_data.next = end_sub + 13;
    return sub_data;
}

//<option>name_string</option>
//<vector>name_string</vector>
//<flag>name_string</flag>
/** generate additional options based on a string config*/
std::size_t FuzzApp::add_custom_options(CLI::App *app, const std::string &description_string) {
    std::size_t current_index{0};
    while(description_string.size() - 5 > current_index && description_string[current_index] == '<') {
        if(description_string.compare(current_index, 7, "<option") == 0) {
            auto end_option = description_string.find("</option>", current_index + 8);
            if(end_option == std::string::npos) {
                break;
            }
            auto header_close = description_string.find_last_of('>', end_option);
            if(header_close == std::string::npos || header_close < current_index) {
                break;
            }
            std::string name = description_string.substr(header_close + 1, end_option - header_close - 1);
            custom_string_options.push_back(std::make_shared<std::pair<std::string, bool>>("", true));
            auto *opt = app->add_option(name, custom_string_options.back()->first);
            if(header_close > current_index + 19) {
                std::string attributes = description_string.substr(current_index + 8, header_close - 8 - current_index);
                modify_option(opt, attributes);
                if(!opt->get_configurable()) {
                    custom_string_options.back()->second = false;
                    if(opt->get_required()) {
                        non_config_required = true;
                    }
                }
            }

            current_index = end_option + 9;
        } else if(description_string.compare(current_index, 5, "<flag") == 0) {
            auto end_option = description_string.find("</flag>", current_index + 6);
            if(end_option == std::string::npos) {
                break;
            }
            auto header_close = description_string.find_last_of('>', end_option);
            if(header_close == std::string::npos || header_close < current_index) {
                break;
            }
            std::string name = description_string.substr(header_close + 1, end_option - header_close - 1);
            custom_string_options.push_back(std::make_shared<std::pair<std::string, bool>>("", true));
            auto *opt = app->add_option(name, custom_string_options.back()->first);

            if(header_close > current_index + 17) {
                std::string attributes = description_string.substr(current_index + 6, header_close - 6 - current_index);
                modify_option(opt, attributes);
                if(!opt->get_configurable()) {
                    custom_string_options.back()->second = false;
                    if(opt->get_required()) {
                        non_config_required = true;
                    }
                }
            }
            current_index = end_option + 7;
        } else if(description_string.compare(current_index, 7, "<vector") == 0) {
            auto end_option = description_string.find("</vector>", current_index + 8);
            if(end_option == std::string::npos) {
                break;
            }
            auto header_close = description_string.find_last_of('>', end_option);
            if(header_close == std::string::npos || header_close < current_index) {
                break;
            }
            std::string name = description_string.substr(header_close + 1, end_option - header_close - 1);
            custom_vector_options.push_back(std::make_shared<std::pair<std::vector<std::string>, bool>>());
            custom_vector_options.back()->second = true;
            auto *opt = app->add_option(name, custom_vector_options.back()->first);
            if(header_close > current_index + 19) {
                std::string attributes = description_string.substr(current_index + 8, header_close - 8 - current_index);
                modify_option(opt, attributes);
                if(!opt->get_configurable()) {
                    custom_vector_options.back()->second = false;
                    if(opt->get_required()) {
                        non_config_required = true;
                    }
                }
            }
            current_index = end_option + 9;
        } else if(description_string.compare(current_index, 11, "<subcommand") == 0) {
            auto end_sub_label = description_string.find_first_of('>', current_index + 12);
            if(end_sub_label == std::string::npos) {
                break;
            }
            auto end_sub = description_string.find("</subcommand>", end_sub_label + 1);
            if(end_sub == std::string::npos) {
                break;
            }
            auto subdata = extract_subcomand_info(description_string, current_index);
            if(subdata.data.empty()) {
                break;
            }
            auto *sub = app->add_subcommand(subdata.name, subdata.description);
            if(!subdata.modifiers.empty()) {
                modify_subcommand(sub, subdata.modifiers);
            }
            add_custom_options(sub, subdata.data);
            current_index = subdata.next;
        } else {
            if(isspace(description_string[current_index]) != 0) {
                ++current_index;
            } else {
                break;
            }
        }
    }
    return current_index;
}

}  // namespace CLI
