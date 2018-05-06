#include "CLI/Timer.hpp"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <chrono>
#include <sstream>
#include <string>
#include <thread>

using ::testing::HasSubstr;

TEST(Timer, MSTimes) {
    CLI::Timer timer{"My Timer"};
    std::this_thread::sleep_for(std::chrono::milliseconds(123));
    std::string output = timer.to_string();
    std::string new_output = (timer / 1000000).to_string();
    EXPECT_THAT(output, HasSubstr("My Timer"));
    EXPECT_THAT(output, HasSubstr(" ms"));
    EXPECT_THAT(new_output, HasSubstr(" ns"));
}

/* Takes too long
TEST(Timer, STimes) {
    CLI::Timer timer;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string output = timer.to_string();
    EXPECT_THAT(output, HasSubstr(" s"));
}
*/

// Fails on Windows
// TEST(Timer, UStimes) {
//    CLI::Timer timer;
//    std::this_thread::sleep_for(std::chrono::microseconds(2));
//    std::string output = timer.to_string();
//    EXPECT_THAT(output, HasSubstr(" ms"));
//}

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

TEST(Timer, TimeItTimer) {
    CLI::Timer timer;
    std::string output = timer.time_it([]() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); }, .1);
    std::cout << output << std::endl;
    EXPECT_THAT(output, HasSubstr("ms"));
}
