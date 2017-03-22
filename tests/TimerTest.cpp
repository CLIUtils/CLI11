#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "CLI/Timer.hpp"
#include <string>
#include <chrono>
#include <thread>
#include <sstream>

using ::testing::HasSubstr;

TEST(Timer, MSTimes) {
    CLI::Timer timer{"My Timer"};
    std::this_thread::sleep_for(std::chrono::milliseconds(123));
    std::string output = timer.to_string();
    EXPECT_THAT(output, HasSubstr("My Timer"));
    EXPECT_THAT(output, HasSubstr(" ms"));
}

/* Takes too long
TEST(Timer, STimes) {
    CLI::Timer timer;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string output = timer.to_string();
    EXPECT_THAT(output, HasSubstr(" s"));
}
*/

TEST(Timer, NStimes) {
    CLI::Timer timer;
    std::string output = timer.to_string();
    EXPECT_THAT(output, HasSubstr(" ns"));
}

TEST(Timer, BigTimer) {
    CLI::Timer timer{"My Timer", CLI::Timer::Big};
    std::string output = timer.to_string();
    EXPECT_THAT(output, HasSubstr("Time ="));
    EXPECT_THAT(output, HasSubstr("-----------"));
}

TEST(Timer, AutoTimer) {
    CLI::AutoTimer timer;
    std::string output = timer.to_string();
    EXPECT_THAT(output, HasSubstr("Timer"));
}

TEST(Timer, PrintTimer) {
    std::stringstream out;
    CLI::AutoTimer timer;
    out << timer;
    std::string output = out.str();
    EXPECT_THAT(output, HasSubstr("Timer"));
}
