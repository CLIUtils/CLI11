#include <array>

#include <CLI/CLI.hpp>

// from @gourin https://github.com/CLIUtils/CLI11/issues/1135
int main(int argc, char** argv)
{
   std::array<int, 2> a{ 0, 1};
   CLI::App app{"My app"};
   app.add_option("--a", a, "an array")->capture_default_str();
   app.parse(argc, argv);

    std::cout<<"pass\n";
    return 0;
}
