#pragma once

// Distributed under the LGPL version 3.0 license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <exception>
#include <stdexcept>

namespace CLI {

// Error definitions


struct Error : public std::runtime_error {
    int exit_code;
    bool print_help;
    Error(std::string parent, std::string name, int exit_code=255, bool print_help=true) : runtime_error(parent + ": " + name), exit_code(exit_code), print_help(print_help) {}
};

struct Success : public Error {
    Success() : Error("Success", "Successfully completed, should be caught and quit", 0, false) {}
};

struct CallForHelp : public Error {
    CallForHelp() : Error("CallForHelp", "This should be caught in your main function, see examples", 0) {}
};

struct BadNameString : public Error {
    BadNameString(std::string name) : Error("BadNameString", name, 1) {}
};


struct ParseError : public Error {
    ParseError(std::string name) : Error("ParseError", name, 2) {}
};

struct OptionAlreadyAdded : public Error {
    OptionAlreadyAdded(std::string name) : Error("OptionAlreadyAdded", name, 3) {}
};

struct OptionNotFound : public Error {
    OptionNotFound(std::string name) : Error("OptionNotFound", name, 4) {}
};

struct RequiredError : public Error {
    RequiredError(std::string name) : Error("RequiredError", name, 5) {}
};

struct PositionalError : public Error {
    PositionalError(std::string name) : Error("PositionalError", name, 6) {}
};

struct HorribleError : public Error {
    HorribleError(std::string name) : Error("HorribleError", "(You should never see this error) " + name, 7) {}
};
struct IncorrectConstruction : public Error {
    IncorrectConstruction(std::string name) : Error("IncorrectConstruction", name, 8) {}
};
struct EmptyError : public Error {
    EmptyError(std::string name) : Error("EmptyError", name, 9) {}
};

}
