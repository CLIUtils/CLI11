#include "CLI/CLI.hpp"

int main(int argc, char **argv) {

    CLI::App app("Validator checker");

    std::string file;
    app.add_option("-f,--file,file", file, "File name")->check(CLI::ExistingFile);

    int count;
    app.add_option("-v,--value", count, "Value in range")->check(CLI::Range(3, 6));
    CLI11_PARSE(app, argc, argv);

    std::cout << "Try printing help or failing the validator" << std::endl;

    return 0;
}
