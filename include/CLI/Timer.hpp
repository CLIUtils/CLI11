#pragma once

// Distributed under the LGPL v2.1 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <iostream>
#include <chrono>
#include <functional>

namespace CLI {

class Timer {
protected:
    typedef std::chrono::high_resolution_clock clock;
    typedef std::chrono::time_point<clock> time_point;
    typedef std::function<std::string(std::string, std::string)> time_print_t;

    std::string title_;
    time_point start_;
    time_print_t time_print_;
public:
    static std::string Simple(std::string title, std::string time) {
        return title + ": " + time;
    }

    static std::string Big(std::string title, std::string time) {
        return "-----------------------------------------\n"
            + title + " | Time = " + time + "\n"
            + "-----------------------------------------";
    }

public:
    Timer(std::string title="Timer", time_print_t time_print = Simple)
        : title_(title), time_print_(time_print), start_(clock::now()) {}


    std::string make_time_str() const {
        time_point stop = clock::now();
        std::chrono::duration<double, std::milli> elapsed = stop - start_;
        double time = elapsed.count();
        
        // LCOV_EXCL_START
        if(time < 1)
            return std::to_string(int(time*1000000)) + " ns";
        else if(time < 10)
            return std::to_string(int(time*100) / 100.) + " ms";
        else if(time < 1000)
            return std::to_string(int(time)) + " ms";
        else if(time < 10000)
            return std::to_string(int(time*10000) / 10. ) + " s";
        else
            return std::to_string(int(time*1000)) + " s";
        // LCOV_EXCL_END
    }
    std::string to_string() const {
        return time_print_(title_, make_time_str());
    }
};

class AutoTimer : public Timer {
public:
    using Timer::Timer;

    ~AutoTimer () {
        std::cout << to_string() << std::endl;
    }
};

}

std::ostream & operator<< (std::ostream& in, const CLI::Timer& timer) {
    return in << timer.to_string();
}
