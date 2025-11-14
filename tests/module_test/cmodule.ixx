module;

#include <CLI/CLI.hpp>

export module math;

export void foo(CLI::App *app) {

}

export int add(int a, int b){
    return a + b;
}

export int subtract(int a, int b){
    return a - b;
}