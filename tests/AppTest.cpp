#include "app_helper.hpp"

TEST_F(TApp, OneFlagShort) {
    app.add_flag("-c,--count");
    args = {"-c"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(1, app.count("-c"));
    EXPECT_EQ(1, app.count("--count"));
}

TEST_F(TApp, OneFlagLong) {
    app.add_flag("-c,--count");
    args = {"--count"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(1, app.count("-c"));
    EXPECT_EQ(1, app.count("--count"));
}

TEST_F(TApp, DashedOptions) {
    app.add_flag("-c");
    app.add_flag("--q");
    app.add_flag("--this,--that");

    args = {"-c", "--q", "--this", "--that"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(1, app.count("-c"));
    EXPECT_EQ(1, app.count("--q"));
    EXPECT_EQ(2, app.count("--this"));
    EXPECT_EQ(2, app.count("--that"));

}


TEST_F(TApp, OneFlagRef) {
    int ref;
    app.add_flag("-c,--count", ref);
    args = {"--count"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(1, app.count("-c"));
    EXPECT_EQ(1, app.count("--count"));
    EXPECT_EQ(1, ref);
}

TEST_F(TApp, OneString) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string", "mystring"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(1, app.count("-s"));
    EXPECT_EQ(1, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}

TEST_F(TApp, OneStringEqualVersion) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string=mystring"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(1, app.count("-s"));
    EXPECT_EQ(1, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}


TEST_F(TApp, TogetherInt) {
    int i;
    app.add_option("-i,--int", i);
    args = {"-i4"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(1, app.count("--int"));
    EXPECT_EQ(1, app.count("-i"));
    EXPECT_EQ(i, 4);
}

TEST_F(TApp, SepInt) {
    int i;
    app.add_option("-i,--int", i);
    args = {"-i","4"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(1, app.count("--int"));
    EXPECT_EQ(1, app.count("-i"));
    EXPECT_EQ(i, 4);
}

TEST_F(TApp, OneStringAgain) {
    std::string str;
    app.add_option("-s,--string", str);
    args = {"--string", "mystring"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(1, app.count("-s"));
    EXPECT_EQ(1, app.count("--string"));
    EXPECT_EQ(str, "mystring");
}


TEST_F(TApp, DefaultStringAgain) {
    std::string str = "previous";
    app.add_option("-s,--string", str);
    EXPECT_NO_THROW(run());
    EXPECT_EQ(0, app.count("-s"));
    EXPECT_EQ(0, app.count("--string"));
    EXPECT_EQ(str, "previous");
}

TEST_F(TApp, LotsOfFlags) {

    app.add_flag("-a");
    app.add_flag("-A");
    app.add_flag("-b");

    args = {"-a","-b","-aA"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(2, app.count("-a"));
    EXPECT_EQ(1, app.count("-b"));
    EXPECT_EQ(1, app.count("-A"));
}


TEST_F(TApp, BoolAndIntFlags) {

    bool bflag;
    int iflag;
    unsigned int uflag;

    app.add_flag("-b", bflag);
    app.add_flag("-i", iflag);
    app.add_flag("-u", uflag);

    args = {"-b", "-i", "-u"};
    EXPECT_NO_THROW(run());
    EXPECT_TRUE(bflag);
    EXPECT_EQ(1, iflag);
    EXPECT_EQ((unsigned int) 1, uflag);

    app.reset();

    args = {"-b", "-b"};
    EXPECT_THROW(run(), CLI::ConversionError);

    app.reset();
    bflag = false;

    args = {"-iiiuu"};
    EXPECT_NO_THROW(run());
    EXPECT_FALSE(bflag);
    EXPECT_EQ(3, iflag);
    EXPECT_EQ((unsigned int) 2, uflag);
}

TEST_F(TApp, ShortOpts) {

    unsigned long long funnyint;
    std::string someopt;
    app.add_flag("-z", funnyint);
    app.add_option("-y", someopt);

    args = {"-zzyzyz",};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(2, app.count("-z"));
    EXPECT_EQ(1, app.count("-y"));
    EXPECT_EQ((unsigned long long) 2, funnyint);
    EXPECT_EQ("zyz", someopt);
}

TEST_F(TApp, DefaultOpts) {

    int i = 3;
    std::string s = "HI";

    app.add_option("-i,i", i, "", false);
    app.add_option("-s,s", s, "", true);

    args = {"-i2", "9"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(1, app.count("i"));
    EXPECT_EQ(1, app.count("-s"));
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
    EXPECT_NO_THROW(run());

}

TEST_F(TApp, Positionals) {

    std::string posit1;
    std::string posit2;
    app.add_option("posit1", posit1);
    app.add_option("posit2", posit2);

    args = {"thing1","thing2"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(1, app.count("posit1"));
    EXPECT_EQ(1, app.count("posit2"));
    EXPECT_EQ("thing1", posit1);
    EXPECT_EQ("thing2", posit2);
}

TEST_F(TApp, MixedPositionals) {

    int positional_int;
    std::string positional_string;
    app.add_option("posit1,--posit1", positional_int, "");
    app.add_option("posit2,--posit2", positional_string, "");

    args = {"--posit2","thing2","7"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(1, app.count("posit2"));
    EXPECT_EQ(1, app.count("--posit1"));
    EXPECT_EQ(7, positional_int);
    EXPECT_EQ("thing2", positional_string);
}

TEST_F(TApp, Reset) {

    app.add_flag("--simple");
    double doub;
    app.add_option("-d,--double", doub);

    args = {"--simple", "--double", "1.2"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(1, app.count("--simple"));
    EXPECT_EQ(1, app.count("-d"));
    EXPECT_FLOAT_EQ(1.2, doub);

    app.reset();

    EXPECT_EQ(0, app.count("--simple"));
    EXPECT_EQ(0, app.count("-d"));
    
    EXPECT_NO_THROW(run());

    EXPECT_EQ(1, app.count("--simple"));
    EXPECT_EQ(1, app.count("-d"));
    EXPECT_FLOAT_EQ(1.2, doub);

}


TEST_F(TApp, FileNotExists) {
    std::string myfile{"TestNonFileNotUsed.txt"};
    EXPECT_TRUE(CLI::NonexistentPath(myfile));

    std::string filename;
    app.add_option("--file", filename)->check(CLI::NonexistentPath);
    args = {"--file", myfile};

    EXPECT_NO_THROW(run());
    EXPECT_EQ(myfile, filename);

    app.reset();

    
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    EXPECT_THROW(run(), CLI::ConversionError);

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingFile(myfile));
}

TEST_F(TApp, FileExists) {
    std::string myfile{"TestNonFileNotUsed.txt"};
    EXPECT_FALSE(CLI::ExistingFile(myfile));

    std::string filename = "Failed";
    app.add_option("--file", filename)->check(CLI::ExistingFile);
    args = {"--file", myfile};

    EXPECT_THROW(run(), CLI::ConversionError);
    EXPECT_EQ("Failed", filename);

    app.reset();

    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    EXPECT_NO_THROW(run());
    EXPECT_EQ(myfile, filename);

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingFile(myfile));
}

TEST_F(TApp, InSet) {

    std::string choice;
    app.add_set("-q,--quick", choice, {"one", "two", "three"});
    
    args = {"--quick", "two"};

    EXPECT_NO_THROW(run());
    EXPECT_EQ("two", choice);

    app.reset();

    args = {"--quick", "four"};
    EXPECT_THROW(run(), CLI::ConversionError);
}

TEST_F(TApp, VectorFixedString) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option* opt = app.add_option("-s,--string", strvec)->expected(3);
    EXPECT_EQ(3, opt->get_expected());
    
    args = {"--string", "mystring", "mystring2", "mystring3"};
    run();
    EXPECT_EQ(3, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}



TEST_F(TApp, VectorUnlimString) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option* opt = app.add_option("-s,--string", strvec);
    EXPECT_EQ(-1, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(3, app.count("--string"));
    EXPECT_EQ(answer, strvec);
}


TEST_F(TApp, VectorFancyOpts) {
    std::vector<std::string> strvec;
    std::vector<std::string> answer{"mystring", "mystring2", "mystring3"};

    CLI::Option* opt = app.add_option("-s,--string", strvec)->required()->expected(3);
    EXPECT_EQ(3, opt->get_expected());

    args = {"--string", "mystring", "mystring2", "mystring3"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(3, app.count("--string"));
    EXPECT_EQ(answer, strvec);

    app.reset();
    args = {"one", "two"};
    EXPECT_THROW(run(), CLI::RequiredError);

    app.reset();
    EXPECT_THROW(run(), CLI::ParseError);
}

TEST_F(TApp, RequiresFlags) {
    CLI::Option* opt = app.add_flag("-s,--string");
    app.add_flag("--both")->requires(opt);

    EXPECT_NO_THROW(run());

    app.reset();
    args = {"-s"};
    EXPECT_NO_THROW(run());

    app.reset();
    args = {"-s", "--both"};
    EXPECT_NO_THROW(run());

    app.reset();
    args = {"--both"};
    EXPECT_THROW(run(), CLI::RequiresError);
}


TEST_F(TApp, ExcludesFlags) {
    CLI::Option* opt = app.add_flag("-s,--string");
    app.add_flag("--nostr")->excludes(opt);

    EXPECT_NO_THROW(run());

    app.reset();
    args = {"-s"};
    EXPECT_NO_THROW(run());

    app.reset();
    args = {"--nostr"};
    EXPECT_NO_THROW(run());

    app.reset();
    args = {"--nostr", "-s"};
    EXPECT_THROW(run(), CLI::ExcludesError);

    app.reset();
    args = {"--string", "--nostr"};
    EXPECT_THROW(run(), CLI::ExcludesError);
}


TEST_F(TApp, RequiresMultiFlags) {
    CLI::Option* opt1 = app.add_flag("--opt1");
    CLI::Option* opt2 = app.add_flag("--opt2");
    CLI::Option* opt3 = app.add_flag("--opt3");
    app.add_flag("--optall")->requires(opt1, opt2, opt3);

    EXPECT_NO_THROW(run());

    app.reset();
    args = {"--opt1"};
    EXPECT_NO_THROW(run());

    app.reset();
    args = {"--opt2"};
    EXPECT_NO_THROW(run());

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
    EXPECT_NO_THROW(run());
}

TEST_F(TApp, RequiresChainedFlags) {
    CLI::Option* opt1 = app.add_flag("--opt1");
    CLI::Option* opt2 = app.add_flag("--opt2")->requires(opt1);
    app.add_flag("--opt3")->requires(opt2);

    EXPECT_NO_THROW(run());

    app.reset();
    args = {"--opt1"};
    EXPECT_NO_THROW(run());

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
    EXPECT_NO_THROW(run());

    app.reset();
    args = {"--opt1", "--opt2", "--opt3"};
    EXPECT_NO_THROW(run());
}



// TODO: add tests for envname
