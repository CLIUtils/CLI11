#include "app_helper.hpp"

#include <complex>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>

TEST(Split, SimpleByToken) {
    auto out = CLI::detail::split("one.two.three", '.');
    ASSERT_EQ((size_t)3, out.size());
    EXPECT_EQ("one", out.at(0));
    EXPECT_EQ("two", out.at(1));
    EXPECT_EQ("three", out.at(2));
}

TEST(Split, Single) {
    auto out = CLI::detail::split("one", '.');
    ASSERT_EQ((size_t)1, out.size());
    EXPECT_EQ("one", out.at(0));
}

TEST(Split, Empty) {
    auto out = CLI::detail::split("", '.');
    ASSERT_EQ((size_t)1, out.size());
    EXPECT_EQ("", out.at(0));
}

TEST(String, InvalidName) {
    EXPECT_TRUE(CLI::detail::valid_name_string("valid"));
    EXPECT_FALSE(CLI::detail::valid_name_string("-invalid"));
    EXPECT_TRUE(CLI::detail::valid_name_string("va-li-d"));
    EXPECT_FALSE(CLI::detail::valid_name_string("vali&d"));
    EXPECT_TRUE(CLI::detail::valid_name_string("_valid"));
}

TEST(Trim, Various) {
    std::string s1{"  sdlfkj sdflk sd s  "};
    std::string a1{"sdlfkj sdflk sd s"};
    CLI::detail::trim(s1);
    EXPECT_EQ(a1, s1);

    std::string s2{" a \t"};
    CLI::detail::trim(s2);
    EXPECT_EQ("a", s2);

    std::string s3{" a \n"};
    CLI::detail::trim(s3);
    EXPECT_EQ("a", s3);

    std::string s4{" a b "};
    EXPECT_EQ("a b", CLI::detail::trim(s4));
}

TEST(Trim, VariousFilters) {
    std::string s1{"  sdlfkj sdflk sd s  "};
    std::string a1{"sdlfkj sdflk sd s"};
    CLI::detail::trim(s1, " ");
    EXPECT_EQ(a1, s1);

    std::string s2{" a \t"};
    CLI::detail::trim(s2, " ");
    EXPECT_EQ("a \t", s2);

    std::string s3{"abdavda"};
    CLI::detail::trim(s3, "a");
    EXPECT_EQ("bdavd", s3);

    std::string s4{"abcabcabc"};
    EXPECT_EQ("cabcabc", CLI::detail::trim(s4, "ab"));
}

TEST(Trim, TrimCopy) {
    std::string orig{" cabc  "};
    std::string trimmed = CLI::detail::trim_copy(orig);
    EXPECT_EQ("cabc", trimmed);
    EXPECT_NE(orig, trimmed);
    CLI::detail::trim(orig);
    EXPECT_EQ(trimmed, orig);

    orig = "abcabcabc";
    trimmed = CLI::detail::trim_copy(orig, "ab");
    EXPECT_EQ("cabcabc", trimmed);
    EXPECT_NE(orig, trimmed);
    CLI::detail::trim(orig, "ab");
    EXPECT_EQ(trimmed, orig);
}

TEST(Validators, FileExists) {
    std::string myfile{"TestFileNotUsed.txt"};
    EXPECT_FALSE(CLI::ExistingFile(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    EXPECT_TRUE(CLI::ExistingFile(myfile).empty());

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingFile(myfile).empty());
}

TEST(Validators, FileNotExists) {
    std::string myfile{"TestFileNotUsed.txt"};
    EXPECT_TRUE(CLI::NonexistentPath(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    EXPECT_FALSE(CLI::NonexistentPath(myfile).empty());

    std::remove(myfile.c_str());
    EXPECT_TRUE(CLI::NonexistentPath(myfile).empty());
}

TEST(Validators, FileIsDir) {
    std::string mydir{"../tests"};
    EXPECT_NE(CLI::ExistingFile(mydir), "");
}

TEST(Validators, DirectoryExists) {
    std::string mydir{"../tests"};
    EXPECT_EQ(CLI::ExistingDirectory(mydir), "");
}

TEST(Validators, DirectoryNotExists) {
    std::string mydir{"nondirectory"};
    EXPECT_NE(CLI::ExistingDirectory(mydir), "");
}

TEST(Validators, DirectoryIsFile) {
    std::string myfile{"TestFileNotUsed.txt"};
    EXPECT_TRUE(CLI::NonexistentPath(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    EXPECT_FALSE(CLI::ExistingDirectory(myfile).empty());

    std::remove(myfile.c_str());
    EXPECT_TRUE(CLI::NonexistentPath(myfile).empty());
}

TEST(Validators, PathExistsDir) {
    std::string mydir{"../tests"};
    EXPECT_EQ(CLI::ExistingPath(mydir), "");
}

TEST(Validators, PathExistsFile) {
    std::string myfile{"TestFileNotUsed.txt"};
    EXPECT_FALSE(CLI::ExistingPath(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);
    EXPECT_TRUE(CLI::ExistingPath(myfile).empty());

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingPath(myfile).empty());
}

TEST(Validators, PathNotExistsDir) {
    std::string mydir{"nonpath"};
    EXPECT_NE(CLI::ExistingPath(mydir), "");
}

TEST(Validators, CombinedAndRange) {
    auto crange = CLI::Range(0, 12) & CLI::Range(4, 16);
    EXPECT_TRUE(crange("4").empty());
    EXPECT_TRUE(crange("12").empty());
    EXPECT_TRUE(crange("7").empty());

    EXPECT_FALSE(crange("-2").empty());
    EXPECT_FALSE(crange("2").empty());
    EXPECT_FALSE(crange("15").empty());
    EXPECT_FALSE(crange("16").empty());
    EXPECT_FALSE(crange("18").empty());
}

TEST(Validators, CombinedOrRange) {
    auto crange = CLI::Range(0, 4) | CLI::Range(8, 12);

    EXPECT_FALSE(crange("-2").empty());
    EXPECT_TRUE(crange("2").empty());
    EXPECT_FALSE(crange("5").empty());
    EXPECT_TRUE(crange("8").empty());
    EXPECT_TRUE(crange("12").empty());
    EXPECT_FALSE(crange("16").empty());
}

TEST(Validators, CombinedPaths) {
    std::string myfile{"TestFileNotUsed.txt"};
    EXPECT_FALSE(CLI::ExistingFile(myfile).empty());
    bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
    EXPECT_TRUE(ok);

    std::string dir{"../tests"};
    std::string notpath{"nondirectory"};

    auto path_or_dir = CLI::ExistingPath | CLI::ExistingDirectory;
    EXPECT_TRUE(path_or_dir(dir).empty());
    EXPECT_TRUE(path_or_dir(myfile).empty());
    EXPECT_FALSE(path_or_dir(notpath).empty());

    auto file_or_dir = CLI::ExistingFile | CLI::ExistingDirectory;
    EXPECT_TRUE(file_or_dir(dir).empty());
    EXPECT_TRUE(file_or_dir(myfile).empty());
    EXPECT_FALSE(file_or_dir(notpath).empty());

    auto path_and_dir = CLI::ExistingPath & CLI::ExistingDirectory;
    EXPECT_TRUE(path_and_dir(dir).empty());
    EXPECT_FALSE(path_and_dir(myfile).empty());
    EXPECT_FALSE(path_and_dir(notpath).empty());

    auto path_and_file = CLI::ExistingFile & CLI::ExistingDirectory;
    EXPECT_FALSE(path_and_file(dir).empty());
    EXPECT_FALSE(path_and_file(myfile).empty());
    EXPECT_FALSE(path_and_file(notpath).empty());

    std::remove(myfile.c_str());
    EXPECT_FALSE(CLI::ExistingFile(myfile).empty());
}

// Yes, this is testing an app_helper :)
TEST(AppHelper, TempfileCreated) {
    std::string name = "TestFileNotUsed.txt";
    {
        TempFile myfile{name};

        EXPECT_FALSE(CLI::ExistingFile(myfile).empty());

        bool ok = static_cast<bool>(std::ofstream(myfile.c_str()).put('a')); // create file
        EXPECT_TRUE(ok);
        EXPECT_TRUE(CLI::ExistingFile(name).empty());
        EXPECT_THROW({ TempFile otherfile(name); }, std::runtime_error);
    }
    EXPECT_FALSE(CLI::ExistingFile(name).empty());
}

TEST(AppHelper, TempfileNotCreated) {
    std::string name = "TestFileNotUsed.txt";
    {
        TempFile myfile{name};

        EXPECT_FALSE(CLI::ExistingFile(myfile).empty());
    }
    EXPECT_FALSE(CLI::ExistingFile(name).empty());
}

TEST(AppHelper, Ofstream) {

    std::string name = "TestFileNotUsed.txt";
    {
        TempFile myfile(name);

        {
            std::ofstream out{myfile};
            out << "this is output" << std::endl;
        }

        EXPECT_TRUE(CLI::ExistingFile(myfile).empty());
    }
    EXPECT_FALSE(CLI::ExistingFile(name).empty());
}

TEST(Split, StringList) {

    std::vector<std::string> results{"a", "long", "--lone", "-q"};
    EXPECT_EQ(results, CLI::detail::split_names("a,long,--lone,-q"));
    EXPECT_EQ(results, CLI::detail::split_names(" a, long, --lone, -q"));
    EXPECT_EQ(results, CLI::detail::split_names(" a , long , --lone , -q "));
    EXPECT_EQ(results, CLI::detail::split_names("   a  ,  long  ,  --lone  ,    -q  "));

    EXPECT_EQ(std::vector<std::string>({"one"}), CLI::detail::split_names("one"));
}

TEST(RegEx, Shorts) {
    std::string name, value;

    EXPECT_TRUE(CLI::detail::split_short("-a", name, value));
    EXPECT_EQ("a", name);
    EXPECT_EQ("", value);

    EXPECT_TRUE(CLI::detail::split_short("-B", name, value));
    EXPECT_EQ("B", name);
    EXPECT_EQ("", value);

    EXPECT_TRUE(CLI::detail::split_short("-cc", name, value));
    EXPECT_EQ("c", name);
    EXPECT_EQ("c", value);

    EXPECT_TRUE(CLI::detail::split_short("-simple", name, value));
    EXPECT_EQ("s", name);
    EXPECT_EQ("imple", value);

    EXPECT_FALSE(CLI::detail::split_short("--a", name, value));
    EXPECT_FALSE(CLI::detail::split_short("--thing", name, value));
    EXPECT_FALSE(CLI::detail::split_short("--", name, value));
    EXPECT_FALSE(CLI::detail::split_short("something", name, value));
    EXPECT_FALSE(CLI::detail::split_short("s", name, value));
}

TEST(RegEx, Longs) {
    std::string name, value;

    EXPECT_TRUE(CLI::detail::split_long("--a", name, value));
    EXPECT_EQ("a", name);
    EXPECT_EQ("", value);

    EXPECT_TRUE(CLI::detail::split_long("--thing", name, value));
    EXPECT_EQ("thing", name);
    EXPECT_EQ("", value);

    EXPECT_TRUE(CLI::detail::split_long("--some=thing", name, value));
    EXPECT_EQ("some", name);
    EXPECT_EQ("thing", value);

    EXPECT_FALSE(CLI::detail::split_long("-a", name, value));
    EXPECT_FALSE(CLI::detail::split_long("-things", name, value));
    EXPECT_FALSE(CLI::detail::split_long("Q", name, value));
    EXPECT_FALSE(CLI::detail::split_long("--", name, value));
}

TEST(RegEx, SplittingNew) {

    std::vector<std::string> shorts;
    std::vector<std::string> longs;
    std::string pname;

    EXPECT_NO_THROW(std::tie(shorts, longs, pname) = CLI::detail::get_names({"--long", "-s", "-q", "--also-long"}));
    EXPECT_EQ(std::vector<std::string>({"long", "also-long"}), longs);
    EXPECT_EQ(std::vector<std::string>({"s", "q"}), shorts);
    EXPECT_EQ("", pname);

    EXPECT_NO_THROW(std::tie(shorts, longs, pname) =
                        CLI::detail::get_names({"--long", "", "-s", "-q", "", "--also-long"}));
    EXPECT_EQ(std::vector<std::string>({"long", "also-long"}), longs);
    EXPECT_EQ(std::vector<std::string>({"s", "q"}), shorts);

    EXPECT_THROW(std::tie(shorts, longs, pname) = CLI::detail::get_names({"-"}), CLI::BadNameString);
    EXPECT_THROW(std::tie(shorts, longs, pname) = CLI::detail::get_names({"--"}), CLI::BadNameString);
    EXPECT_THROW(std::tie(shorts, longs, pname) = CLI::detail::get_names({"-hi"}), CLI::BadNameString);
    EXPECT_THROW(std::tie(shorts, longs, pname) = CLI::detail::get_names({"---hi"}), CLI::BadNameString);
    EXPECT_THROW(std::tie(shorts, longs, pname) = CLI::detail::get_names({"one", "two"}), CLI::BadNameString);
}

TEST(String, ToLower) { EXPECT_EQ("one and two", CLI::detail::to_lower("one And TWO")); }

TEST(Join, Forward) {
    std::vector<std::string> val{{"one", "two", "three"}};
    EXPECT_EQ("one,two,three", CLI::detail::join(val));
    EXPECT_EQ("one;two;three", CLI::detail::join(val, ";"));
}

TEST(Join, Backward) {
    std::vector<std::string> val{{"three", "two", "one"}};
    EXPECT_EQ("one,two,three", CLI::detail::rjoin(val));
    EXPECT_EQ("one;two;three", CLI::detail::rjoin(val, ";"));
}

TEST(SplitUp, Simple) {
    std::vector<std::string> oput = {"one", "two three"};
    std::string orig{R"(one "two three")"};
    std::vector<std::string> result = CLI::detail::split_up(orig);
    EXPECT_EQ(oput, result);
}

TEST(SplitUp, Layered) {
    std::vector<std::string> output = {R"(one 'two three')"};
    std::string orig{R"("one 'two three'")"};
    std::vector<std::string> result = CLI::detail::split_up(orig);
    EXPECT_EQ(output, result);
}

TEST(SplitUp, Spaces) {
    std::vector<std::string> oput = {"one", "  two three"};
    std::string orig{R"(  one  "  two three" )"};
    std::vector<std::string> result = CLI::detail::split_up(orig);
    EXPECT_EQ(oput, result);
}

TEST(SplitUp, BadStrings) {
    std::vector<std::string> oput = {"one", "  two three"};
    std::string orig{R"(  one  "  two three )"};
    std::vector<std::string> result = CLI::detail::split_up(orig);
    EXPECT_EQ(oput, result);

    oput = {"one", "  two three"};
    orig = R"(  one  '  two three )";
    result = CLI::detail::split_up(orig);
    EXPECT_EQ(oput, result);
}

TEST(Types, TypeName) {
    std::string int_name = CLI::detail::type_name<int>();
    EXPECT_EQ("INT", int_name);

    std::string int2_name = CLI::detail::type_name<short>();
    EXPECT_EQ("INT", int2_name);

    std::string uint_name = CLI::detail::type_name<unsigned char>();
    EXPECT_EQ("UINT", uint_name);

    std::string float_name = CLI::detail::type_name<double>();
    EXPECT_EQ("FLOAT", float_name);

    std::string vector_name = CLI::detail::type_name<std::vector<int>>();
    EXPECT_EQ("VECTOR", vector_name);

    std::string text_name = CLI::detail::type_name<std::string>();
    EXPECT_EQ("TEXT", text_name);

    std::string text2_name = CLI::detail::type_name<char *>();
    EXPECT_EQ("TEXT", text2_name);
}

TEST(Types, OverflowSmall) {
    char x;
    auto strmax = std::to_string(INT8_MAX + 1);
    EXPECT_FALSE(CLI::detail::lexical_cast(strmax, x));

    unsigned char y;
    strmax = std::to_string(UINT8_MAX + 1);
    EXPECT_FALSE(CLI::detail::lexical_cast(strmax, y));
}

TEST(Types, LexicalCastInt) {
    std::string signed_input = "-912";
    int x_signed;
    EXPECT_TRUE(CLI::detail::lexical_cast(signed_input, x_signed));
    EXPECT_EQ(-912, x_signed);

    std::string unsigned_input = "912";
    unsigned int x_unsigned;
    EXPECT_TRUE(CLI::detail::lexical_cast(unsigned_input, x_unsigned));
    EXPECT_EQ((unsigned int)912, x_unsigned);

    EXPECT_FALSE(CLI::detail::lexical_cast(signed_input, x_unsigned));

    unsigned char y;
    std::string overflow_input = std::to_string(UINT64_MAX) + "0";
    EXPECT_FALSE(CLI::detail::lexical_cast(overflow_input, y));

    char y_signed;
    EXPECT_FALSE(CLI::detail::lexical_cast(overflow_input, y_signed));

    std::string bad_input = "hello";
    EXPECT_FALSE(CLI::detail::lexical_cast(bad_input, y));

    std::string extra_input = "912i";
    EXPECT_FALSE(CLI::detail::lexical_cast(extra_input, y));
}

TEST(Types, LexicalCastDouble) {
    std::string input = "9.12";
    long double x;
    EXPECT_TRUE(CLI::detail::lexical_cast(input, x));
    EXPECT_FLOAT_EQ((float)9.12, (float)x);

    std::string bad_input = "hello";
    EXPECT_FALSE(CLI::detail::lexical_cast(bad_input, x));

    std::string overflow_input = "1" + std::to_string(LDBL_MAX);
    EXPECT_FALSE(CLI::detail::lexical_cast(overflow_input, x));

    std::string extra_input = "9.12i";
    EXPECT_FALSE(CLI::detail::lexical_cast(extra_input, x));
}

TEST(Types, LexicalCastString) {
    std::string input = "one";
    std::string output;
    CLI::detail::lexical_cast(input, output);
    EXPECT_EQ(input, output);
}

TEST(Types, LexicalCastParsable) {
    std::string input = "(4.2,7.3)";
    std::string fail_input = "4.2,7.3";
    std::string extra_input = "(4.2,7.3)e";

    std::complex<double> output;
    EXPECT_TRUE(CLI::detail::lexical_cast(input, output));
    EXPECT_DOUBLE_EQ(output.real(), 4.2); // Doing this in one go sometimes has trouble
    EXPECT_DOUBLE_EQ(output.imag(), 7.3); // on clang + c++4.8 due to missing const

    EXPECT_FALSE(CLI::detail::lexical_cast(fail_input, output));
    EXPECT_FALSE(CLI::detail::lexical_cast(extra_input, output));
}

TEST(FixNewLines, BasicCheck) {
    std::string input = "one\ntwo";
    std::string output = "one\n; two";
    std::string result = CLI::detail::fix_newlines("; ", input);
    EXPECT_EQ(result, output);
}

TEST(FixNewLines, EdgesCheck) {
    std::string input = "\none\ntwo\n";
    std::string output = "\n; one\n; two\n; ";
    std::string result = CLI::detail::fix_newlines("; ", input);
    EXPECT_EQ(result, output);
}
