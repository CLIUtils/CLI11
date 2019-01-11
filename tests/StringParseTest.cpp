#include "app_helper.hpp"

#include "gmock/gmock.h"
#include <cstdio>
#include <sstream>

TEST_F(TApp, ExistingExeCheck) {

    TempFile tmpexe{"existingExe.out"};

    std::string str, str2, str3;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);

    {
        std::ofstream out{tmpexe};
        out << "useless string doesn't matter" << std::endl;
    }

    app.parse(std::string("./") + std::string(tmpexe) +
                  " --string=\"this is my quoted string\" -t 'qstring 2' -m=`\"quoted string\"`",
              true);
    EXPECT_EQ(str, "this is my quoted string");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");
}

TEST_F(TApp, ExistingExeCheckWithSpace) {

    TempFile tmpexe{"Space File.out"};

    std::string str, str2, str3;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);

    {
        std::ofstream out{tmpexe};
        out << "useless string doesn't matter" << std::endl;
    }

    app.parse(std::string("./") + std::string(tmpexe) +
                  " --string=\"this is my quoted string\" -t 'qstring 2' -m=`\"quoted string\"`",
              true);
    EXPECT_EQ(str, "this is my quoted string");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");

    EXPECT_EQ(app.get_name(), std::string("./") + std::string(tmpexe));
}

TEST_F(TApp, ExistingExeCheckWithLotsOfSpace) {

    TempFile tmpexe{"this is a weird file.exe"};

    std::string str, str2, str3;
    app.add_option("-s,--string", str);
    app.add_option("-t,--tstr", str2);
    app.add_option("-m,--mstr", str3);

    {
        std::ofstream out{tmpexe};
        out << "useless string doesn't matter" << std::endl;
    }

    app.parse(std::string("./") + std::string(tmpexe) +
                  " --string=\"this is my quoted string\" -t 'qstring 2' -m=`\"quoted string\"`",
              true);
    EXPECT_EQ(str, "this is my quoted string");
    EXPECT_EQ(str2, "qstring 2");
    EXPECT_EQ(str3, "\"quoted string\"");

    EXPECT_EQ(app.get_name(), std::string("./") + std::string(tmpexe));
}
