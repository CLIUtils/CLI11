#include "CLI.hpp"


int main (int argc, char** argv) {

    CLI::App app("K3Pi goofit fitter");

    std::string file;
    app.add_option("f,file", file, "File name");
    
    int count;
    app.add_flag<int>("c,count", count, "File name");

    app.parse(argc, argv);

    std::cout << "Working on file: " << file << std::endl;
    std::cout << "Working on count: " << count << std::endl;

    return 0;
}
