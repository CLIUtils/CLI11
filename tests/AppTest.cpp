#include "app_helper.hpp"
#include <cstdlib>

TEST_F(TApp, OneFlagShort) {
    app.add_flag("-c,--count");
    args = {"-c"};
    run();
    EXPECT_EQ((size_t)1, app.count("-c"));
    EXPECT_EQ((size_t)1, app.count("--count"));
}

TEST_F(TApp, CountNonExist) {
    app.add_flag("-c,--count");
    args = {"-c"};
    run();
    EXPECT_THROW(app.count("--nonexist"), CLI::OptionNotFound);
}

TEST_F(TApp, OneFlagLong) {
    app.add_flag("-c,--count");
    args = {"--count"};
    run();
    EXPECT_EQ((size_t)1, app.count("-c"));
    EXPECT_EQ((size_t)1, app.count("--count"));
}

TEST_F(TApp, DashedOptions) {
    app.add_flag("-c");
    app.add_flag("--q");
    app.add_flag("--this,--that");

    args = {"-c", "--q", "--this", "--that"};
    run();
    EXPECT_EQ((size_t)1, app.count("-c"));
    EXPECT_EQ((size_t)1, app.count("--q"));
    EXPECT_EQ((size_t)2, app.count("--this"));
    EXPECT_EQ((size_t)2, app.count("--that"));
}

TEST_F(TApp, OneFlagRef) {
    int ref;
    app.add_flag("-c,--count", ref);
    args = {"--count"};
    run();
    EXPECT_EQ((size_t)1, app.count("-c"));
    EXPECT_EQ((size_t)1, app.count("--count"));
    EXPECT_EQ(1, ref);
}

TEST_F(TApp, OneString) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string", "mystring"};
    run();
    EXPECT_EQ((size_t)1, app.count("-s"));
    EXPECT_EQ((size_t)1, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringEqualVersion) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string=mystring"};
    run();
    EXPECT_EQ((size_t)1, app.count("-s"));
    EXPECT_EQ((size_t)1, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, TogetherInt) {
    int i;
    app.add_option("-i,--int", i);
    args = {"-i4"};
    run();
    EXPECT_EQ((size_t)1, app.count("--int"));
    EXPECT_EQ((size_t)1, app.count("-i"));
    EXPECT_EQ(i, 4);
}

TEST_F(TApp, SepInt) {
    int i;
    app.add_option("-i,--int", i);
    args = {"-i", "4"};
    run();
    EXPECT_EQ((size_t)1, app.count("--int"));
    EXPECT_EQ((size_t)1, app.count("-i"));
    EXPECT_EQ(i, 4);
}

TEST_F(TApp, OneStringAgain) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string", "mystring"};
    run();
    EXPECT_EQ((size_t)1, app.count("-s"));
    EXPECT_EQ((size_t)1, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, DefaultStringAgain) {
    std::string str = "previous";
    app.add_option("-s,--string", str);
    run();
    EXPECT_EQ((size_t)0, app.count("-s"));
    EXPECT_EQ((size_t)0, app.count("--string"));
    EXPECT_EQ(str, "previous");
}

TEST_F(TApp, DualOptions) {

    std::string str = "previous";
    std::vector<std::string> vstr = {"previous"};
    std::vector<std::string> ans = {"one", "two"};
    app.add_option("-s,--string", str);
    app.add_option("-v,--vector", vstr);

    args = {"--vector=one", "--vector=two"};
    run();
    EXPECT_EQ(ans, vstr);

    args = {"--string=one", "--string=two"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, LotsOfFlags) {

    app.add_flag("-a");
    app.add_flag("-A");
    app.add_flag("-b");

    args = {"-a", "-b", "-aA"};
    run();
    EXPECT_EQ((size_t)2, app.count("-a"));
    EXPECT_EQ((size_t)1, app.count("-b"));
    EXPECT_EQ((size_t)1, app.count("-A"));
}

TEST_F(TApp, BoolAndIntFlags) {

    bool bflag;
    int iflag;
    unsigned int uflag;

    app.add_flag("-b", bflag);
    app.add_flag("-i", iflag);
    app.add_flag("-u", uflag);

    args = {"-b", "-i", "-u"};
    run();
    EXPECT_TRUE(bflag);
    EXPECT_EQ(1, iflag);
    EXPECT_EQ((unsigned int)1, uflag);

    app.reset();

    args = {"-b", "-b"};
    EXPECT_THROW(run(), CLI::ConversionError);

    app.reset();
    bflag = false;

    args = {"-iiiuu"};
    run();
    EXPECT_FALSE(bflag);
    EXPECT_EQ(3, iflag);
    EXPECT_EQ((unsigned int)2, uflag);
}

TEST_F(TApp, ShortOpts) {

    unsigned long long funnyint;
    std::string someopt;
    app.add_flag("-z", funnyint);
    app.add_option("-y", someopt);

    args = {
        "-zzyzyz",
    };

    run();

    EXPECT_EQ((size_t)2, app.count("-z"));
    EXPECT_EQ((size_t)1, app.count("-y"));
    EXPECT_EQ((unsigned long long)2, funnyint);
    EXPECT_EQ("zyz", someopt);
}

TEST_F(TApp, DefaultOpts) {

    int i = 3;
    std::string s = "HI";

    app.add_option("-i,i", i, "", false);
    app.add_option("-s,s", s, "", true);

    args = {"-i2", "9"};

    run();

    EXPECT_EQ((size_t)1, app.count("i"));
    EXPECT_EQ((size_t)1, app.count("-s"));
    EXPECT_EQ(2, i);
    EXPECT_EQ("9", s);
}

TEST_F(TApp, RequiredFlags) {
    app.add_flag("-a")->required();
    app.add_flag("-b")->mandatory(); // Alternate term

    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();

    args = {"-a"};
    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();
    args = {"-b"};
    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();
    args = {"-a", "-b"};
    run();
}

TEST_F(TApp, Positionals) {

    std::string posit1;
    std::string posit2;
    app.add_option("posit1", posit1);
    app.add_option("posit2", posit2);

    args = {"thing1", "thing2"};

    run();

    EXPECT_EQ((size_t)1, app.count("posit1"));
    EXPECT_EQ((size_t)1, app.count("posit2"));
    EXPECT_EQ("thing1", posit1);
    EXPECT_EQ("thing2", posit2);
}

TEST_F(TApp, ForcedPositional) {
    std::vector<std::string> posit;
    auto one = app.add_flag("--one");
    app.add_option("posit", posit);

    args = {"--one", "two", "three"};
    run();
    std::vector<std::string> answers1 = {"two", "three"};
    EXPECT_TRUE(one->count());
    EXPECT_EQ(answers1, posit);

    app.reset();

    args = {"--", "--one", "two", "three"};
    std::vector<std::string> answers2 = {"--one", "two", "three"};
    run();

    EXPECT_FALSE(one->count());
    EXPECT_EQ(answers2, posit);
}

TEST_F(TApp, MixedPositionals) {

    int positional_int;
    std::string positional_string;
    app.add_option("posit1,--posit1", positional_int, "");
    app.add_option("posit2,--posit2", positional_string, "");

    args = {"--posit2", "thing2", "7"};

    run();

    EXPECT_EQ((size_t)1, app.count("posit2"));
    EXPECT_EQ((size_t)1, app.count("--posit1"));
    EXPECT_EQ(7, positional_int);
    EXPECT_EQ("thing2", positional_string);
}

TEST_F(TApp, BigPositional) {
    std::vector<std::string> vec;
    app.add_option("pos", vec);

    args = {"one"};

    run();
    EXPECT_EQ(args, vec);

    app.reset();

    args = {"one", "two"};
    run();

    EXPECT_EQ(args, vec);
}

TEST_F(TApp, Reset) {

    app.add_flag("--simple");
    double doub;
    app.add_option("-d,--double", doub);

    args = {"--simple", "--double", "1.2"};

    run();

    EXPECT_EQ((size_t)1, app.count("--simple"));
    EXPECT_EQ((size_t)1, app.count("-d"));
    EXPECT_DOUBLE_EQ(1.2, doub);

    app.reset();

    EXPECT_EQ((size_t)0, app.count("--simple"));
    EXPECT_EQ((size_t)0, app.count("-d"));

    run();

    EXPECT_EQ((size_t)1, app.count("--simple"));
    EXPECT_EQ((size_t)1, app.count("-d"));
    EXPECT_DOUBLE_EQ(1.2, doub);
}

TEST_F(TApp, RemoveOption) {
    app.add_flag("--one");
    auto opt = app.add_flag("--two");

    EXPECT_TRUE(app.remove_option(opt));
    EXPECT_FALSE(app.remove_option(opt));

    args = {"--two"};

    EXPECT_THROW(run(), CLI::ExtrasError);
}

TEST_F(TApp, FileNotExists) {
    std::string myfile{"TestNonFileNotUsed.txt"};
    EXPECT_TRUE(CLI::NonexistentPath(myfile));

    std::string filename;
    app.add_option("--file", filename)->check(CLI::NonexistentPath);
    args = {"--file", myfile};

    run();
    EXPECT_EQ(myfile, filename);

    app.reset();

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    EXPECT_THROW(run(), CLI::ValidationError);

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingFile(myfile));
}

TEST_F(TApp, FileExists) {
    std::string myfile{"TestNonFileNotUsed.txt"};
    EXPECT_FALSE(CLI::ExistingFile(myfile));

    std::string filename = "Failed";
    app.add_option("--file", filename)->check(CLI::ExistingFile);
    args = {"--file", myfile};

    EXPECT_THROW(run(), CLI::ValidationError);

    app.reset();

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    run();
    EXPECT_EQ(myfile, filename);

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingFile(myfile));
}

TEST_F(TApp, InSet) {

    std::string choice;
    app.add_set("-q,--quick", choice, {"one", "two", "three"});

    args = {"--quick", "two"};

    run();
    EXPECT_EQ("two", choice);

    app.reset();

    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, InIntSet) {

    int choice;
    app.add_set("-q,--quick", choice, {1, 2, 3});

    args = {"--quick", "2"};

    run();
    EXPECT_EQ(2, choice);

    app.reset();

    args = {"--quick", "4"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, FailSet) {

    int choice;
    app.add_set("-q,--quick", choice, {1, 2, 3});

    args = {"--quick", "3", "--quick=2"};
    EXPECT_THROW(run(), CLI::ConversionError);

    app.reset();

    args = {"--quick=hello"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, InSetIgnoreCase) {

    std::string choice;
    app.add_set_ignore_case("-q,--quick", choice, {"one", "Two", "THREE"});

    args = {"--quick", "One"};
    run();
    EXPECT_EQ("one", choice);

    app.reset();
    args = {"--quick", "two"};
    run();
    EXPECT_EQ("Two", choice); // Keeps caps from set

    app.reset();
    args = {"--quick", "ThrEE"};
    run();
    EXPECT_EQ("THREE", choice); // Keeps caps from set

    app.reset();
    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ConversionError);

    app.reset();
    args = {"--quick=one", "--quick=two"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, VectorFixedString) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec)->expected(3);
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ((size_t)3, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

TEST_F(TApp, VectorUnlimString) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec);
    EXPECT_EQ(-1, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ((size_t)3, app.count("--string"));
    EXPECT_EQ(answer, strvec);

    app.reset();
    args = {"-s", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ((size_t)3, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}

TEST_F(TApp, VectorFancyOpts) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option *opt = app.add_option("-s,--string", strvec)->required()->expected(3);
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ((size_t)3, app.count("--string"));
    EXPECT_EQ(answer, strvec);

    app.reset();
    args = {"one", "two"};
    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();
    EXPECT_THROW(run(), CLI::ParseError);
}

TEST_F(TApp, RequiresFlags) {
    CLI::Option *opt = app.add_flag("-s,--string");
    app.add_flag("--both")->requires(opt);

    run();

    app.reset();
    args = {"-s"};
    run();

    app.reset();
    args = {"-s", "--both"};
    run();

    app.reset();
    args = {"--both"};
    EXPECT_THROW(run(), CLI::RequiresError);
}

TEST_F(TApp, ExcludesFlags) {
    CLI::Option *opt = app.add_flag("-s,--string");
    app.add_flag("--nostr")->excludes(opt);

    run();

    app.reset();
    args = {"-s"};
    run();

    app.reset();
    args = {"--nostr"};
    run();

    app.reset();
    args = {"--nostr", "-s"};
    EXPECT_THROW(run(), CLI::ExcludesError);

    app.reset();
    args = {"--string", "--nostr"};
    EXPECT_THROW(run(), CLI::ExcludesError);
}

TEST_F(TApp, ExcludesMixedFlags) {
    CLI::Option *opt1 = app.add_flag("--opt1");
    app.add_flag("--opt2");
    CLI::Option *opt3 = app.add_flag("--opt3");
    app.add_flag("--no")->excludes(opt1, "--opt2", opt3);

    run();

    app.reset();
    args = {"--no"};
    run();

    app.reset();
    args = {"--opt2"};
    run();

    app.reset();
    args = {"--no", "--opt1"};
    EXPECT_THROW(run(), CLI::ExcludesError);

    app.reset();
    args = {"--no", "--opt2"};
    EXPECT_THROW(run(), CLI::ExcludesError);
}

TEST_F(TApp, RequiresMultiFlags) {
    CLI::Option *opt1 = app.add_flag("--opt1");
    CLI::Option *opt2 = app.add_flag("--opt2");
    CLI::Option *opt3 = app.add_flag("--opt3");
    app.add_flag("--optall")->requires(opt1, opt2, opt3);

    run();

    app.reset();
    args = {"--opt1"};
    run();

    app.reset();
    args = {"--opt2"};
    run();

    app.reset();
    args = {"--optall"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--optall", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--optall", "--opt2", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--optall", "--opt1", "--opt2", "--opt3"};
    run();
}

TEST_F(TApp, RequiresMixedFlags) {
    CLI::Option *opt1 = app.add_flag("--opt1");
    app.add_flag("--opt2");
    app.add_flag("--opt3");
    app.add_flag("--optall")->requires(opt1, "--opt2", "--opt3");

    run();

    app.reset();
    args = {"--opt1"};
    run();

    app.reset();
    args = {"--opt2"};
    run();

    app.reset();
    args = {"--optall"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--optall", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--optall", "--opt2", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--optall", "--opt1", "--opt2", "--opt3"};
    run();
}

TEST_F(TApp, RequiresChainedFlags) {
    CLI::Option *opt1 = app.add_flag("--opt1");
    CLI::Option *opt2 = app.add_flag("--opt2")->requires(opt1);
    app.add_flag("--opt3")->requires(opt2);

    run();

    app.reset();
    args = {"--opt1"};
    run();

    app.reset();
    args = {"--opt2"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--opt3"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--opt3", "--opt2"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--opt3", "--opt1"};
    EXPECT_THROW(run(), CLI::RequiresError);

    app.reset();
    args = {"--opt2", "--opt1"};
    run();

    app.reset();
    args = {"--opt1", "--opt2", "--opt3"};
    run();
}

TEST_F(TApp, Env) {

    put_env("CLI11_TEST_ENV_TMP", "2");

    int val = 1;
    CLI::Option *vopt = app.add_option("--tmp", val)->envname("CLI11_TEST_ENV_TMP");

    run();

    EXPECT_EQ(2, val);
    EXPECT_EQ((size_t)1, vopt->count());

    app.reset();
    vopt->required();
    run();

    app.reset();
    unset_env("CLI11_TEST_ENV_TMP");
    EXPECT_THROW(run(), CLI::RequiredError);
}

TEST_F(TApp, RangeInt) {
    int x = 0;
    app.add_option("--one", x)->check(CLI::Range(3, 6));

    args = {"--one=1"};
    EXPECT_THROW(run(), CLI::ValidationError);

    app.reset();
    args = {"--one=7"};
    EXPECT_THROW(run(), CLI::ValidationError);

    app.reset();
    args = {"--one=3"};
    run();

    app.reset();
    args = {"--one=5"};
    run();

    app.reset();
    args = {"--one=6"};
    run();
}

TEST_F(TApp, RangeDouble) {

    double x = 0;
    /// Note that this must be a double in Range, too
    app.add_option("--one", x)->check(CLI::Range(3.0, 6.0));

    args = {"--one=1"};
    EXPECT_THROW(run(), CLI::ValidationError);

    app.reset();
    args = {"--one=7"};
    EXPECT_THROW(run(), CLI::ValidationError);

    app.reset();
    args = {"--one=3"};
    run();

    app.reset();
    args = {"--one=5"};
    run();

    app.reset();
    args = {"--one=6"};
    run();
}

// Check to make sure progromatic access to left over is available
TEST_F(TApp, AllowExtras) {

    app.allow_extras();

    bool val = true;
    app.add_flag("-f", val);
    EXPECT_FALSE(val);

    args = {"-x", "-f"};
    std::vector<std::string> left_over;
    EXPECT_NO_THROW({ left_over = run(); });
    EXPECT_TRUE(val);
    EXPECT_EQ(std::vector<std::string>({"-x"}), left_over);
}

TEST_F(TApp, AllowExtrasOrder) {

    app.allow_extras();

    args = {"-x", "-f"};
    std::vector<std::string> left_over;
    EXPECT_NO_THROW({ left_over = run(); });
    EXPECT_EQ(std::vector<std::string>({"-f", "-x"}), left_over);
    app.reset();

    std::vector<std::string> left_over_2;
    left_over_2 = app.parse(left_over);
    EXPECT_EQ(left_over, left_over_2);
}

// Test horrible error
TEST_F(TApp, CheckShortFail) {
    args = {"--two"};

    EXPECT_THROW(CLI::detail::AppFriend::parse_short(&app, args), CLI::HorribleError);
}

// Test horrible error
TEST_F(TApp, CheckLongFail) {
    args = {"-t"};

    EXPECT_THROW(CLI::detail::AppFriend::parse_long(&app, args), CLI::HorribleError);
}

// Test horrible error
TEST_F(TApp, CheckSubcomFail) {
    args = {"subcom"};

    EXPECT_THROW(CLI::detail::AppFriend::parse_subcommand(&app, args), CLI::HorribleError);
}
