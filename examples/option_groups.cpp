#include "CLI/CLI.hpp"

int main(int argc, char **argv) {

    CLI::App app("data output specification");
    app.set_help_all_flag("--help-all", "Expand all help");

    auto format = app.add_option_group("output_format", "formatting type for output");
    auto target = app.add_option_group("output target", "target location for the output");
    bool csv = false;
    bool human = false;
    bool binary = false;
    format->add_flag("--csv", csv, "specify the output in csv format");
    format->add_flag("--human", human, "specify the output in human readable text format");
    format->add_flag("--binary", binary, "specify the output in binary format");
    // require one of the options to be selected
    format->require_option(1);
    std::string fileLoc;
    std::string networkAddress;
    target->add_option("-o,--file", fileLoc, "specify the file location of the output");
    target->add_option("--address", networkAddress, "specify a network address to send the file");

    // require at most one of the target options
    target->require_option(0, 1);
    CLI11_PARSE(app, argc, argv);

    std::string format_type = (csv) ? std::string("CSV") : ((human) ? "human readable" : "binary");
    std::cout << "Selected  " << format_type << "format" << std::endl;
    if(fileLoc.empty()) {
        std::cout << " sent to file " << fileLoc << std::endl;
    } else if(networkAddress.empty()) {
        std::cout << " sent over network to " << networkAddress << std::endl;
    } else {
        std::cout << " sent to std::cout" << std::endl;
    }

    return 0;
}
