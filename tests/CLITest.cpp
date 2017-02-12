#ifdef CLI_SINGLE_FILE
#include "CLI11.hpp"
#else
#include "CLI/CLI.hpp"
#endif

#include "gtest/gtest.h"
#include <fstream>

typedef std::vector<std::string> input_t;

TEST(Basic, Empty) {

    {
    CLI::App app;
    input_t simpleput;
    app.parse(simpleput);
    }
    {
    CLI::App app;
    input_t spare = {"spare"};
    EXPECT_THROW(app.parse(spare), CLI::PositionalError);
    }
    {
    CLI::App app;
    input_t simpleput;
    app.parse(simpleput);
    }
}

struct TApp : public ::testing::Test {
    CLI::App app{"My Test Program"};
    input_t args;

    void run() {
        input_t newargs = args;
        std::reverse(std::begin(newargs), std::end(newargs));
        app.parse(newargs);
    }

};

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

TEST_F(TApp, Flags) {

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

struct TIni : public TApp {

    std::ofstream f{"IniParseSimple.ini"};

    void run() {
        f.close();
        TApp::run();
    }

    ~TIni() {
        f.close();
        std::remove("IniParseSimple.ini");
    }

};



TEST_F(TIni, IniParseSimple) {

    int x;
    std::string y;
    
    app.add_option("--something", x);
    app.add_option("--else", y);

    app.add_config("--config","", "", true);

    args = {"--config=IniParseSimple.ini"};


    ASSERT_TRUE(f.good());

    f << "[default]" << std::endl;
    f << "" << std::endl;
    f << "something=7" << std::endl;
    f << "else=seven" << std::endl;

    //EXPECT_NO_THROW
        (run());

    EXPECT_EQ(7, x);
    EXPECT_EQ("seven", y);
}


TEST(Ini, IniDoubleAdd) {

    CLI::App app;

    app.add_config("--first");
    app.add_config("--second");

    EXPECT_NO_THROW(app.count("--second"));
    EXPECT_THROW(app.count("--first"), CLI::OptionNotFound);

}
TEST_F(TApp, BasicSubcommands) {
    auto sub1 = app.add_subcommand("sub1");
    auto sub2 = app.add_subcommand("sub2");

    EXPECT_NO_THROW(run());
    EXPECT_EQ(nullptr, app.get_subcommand());
    
    app.reset();
    args = {"sub1"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(sub1, app.get_subcommand());

    app.reset();
    EXPECT_EQ(nullptr, app.get_subcommand());

    args = {"sub2"};
    EXPECT_NO_THROW(run());
    EXPECT_EQ(sub2, app.get_subcommand());
}


TEST_F(TApp, Callbacks) {
    auto sub1 = app.add_subcommand("sub1");
    sub1->set_callback([](){
            throw CLI::Success();
            });
    auto sub2 = app.add_subcommand("sub2");
    bool val = false;
    sub2->set_callback([&val](){
            val = true;
            });
    
    app.reset();
    args = {"sub2"};
    EXPECT_FALSE(val);
    EXPECT_NO_THROW(run());
    EXPECT_TRUE(val);

}

// TODO: Add directory test



struct SubcommandProgram : public TApp {

    CLI::App* start;
    CLI::App* stop;

    int dummy;
    std::string file;
    int count;

    SubcommandProgram() {
        start = app.add_subcommand("start", "Start prog");
        stop = app.add_subcommand("stop", "Stop prog");
            
        app.add_flag("-d", dummy, "My dummy var");
        start->add_option("-f,--file", file, "File name");
        stop->add_flag("-c,--count", count, "Some flag opt");
    }
};

TEST_F(SubcommandProgram, Working) {
    args = {"-d", "start", "-ffilename"};

    EXPECT_NO_THROW(run());

    EXPECT_EQ(1, dummy);
    EXPECT_EQ(start, app.get_subcommand());
    EXPECT_EQ("filename", file);
}


TEST_F(SubcommandProgram, Spare) {
    args = {"extra", "-d", "start", "-ffilename"};

    EXPECT_THROW(run(), CLI::PositionalError);
}

TEST_F(SubcommandProgram, SpareSub) {
    args = {"-d", "start", "spare", "-ffilename"};

    EXPECT_THROW(run(), CLI::PositionalError);
}


// TODO: add tests for requires, excludes, envname
