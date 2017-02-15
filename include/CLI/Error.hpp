#pragma once

// Distributed under the MIT license.  See accompanying
// file LICENSE or https://github.com/henryiii/CLI11 for details.

#include <string>
#include <exception>
#include <stdexcept>

namespace CLI {

// Error definitions

/// @defgroup error_group Errors
/// @brief Errors thrown by CLI11
///
/// These are the errors that can be thrown. Some of them, like CLI::Success, are not really errors.
/// @{

/// All errors derive from this one
struct Error : public std::runtime_error {
    int exit_code;
    bool print_help;
    Error(std::string parent, std::string name, int exit_code=255, bool print_help=true) : runtime_error(parent + ": " + name), exit_code(exit_code), print_help(print_help) {}
};

/// Construction errors (not in parsing)
struct ConstructionError : public Error {
    // Using Error::Error constructors seem to not work on GCC 4.7
    ConstructionError(std::string parent, std::string name, int exit_code=255, bool print_help=true) : Error(parent, name, exit_code, print_help) {}
};

/// Thrown when an option is set to conflicting values (non-vector and multi args, for example)
struct IncorrectConstruction : public ConstructionError {
    IncorrectConstruction(std::string name) : ConstructionError("IncorrectConstruction", name, 8) {}
};

/// Thrown on construction of a bad name
struct BadNameString : public ConstructionError {
    BadNameString(std::string name) : ConstructionError("BadNameString", name, 1) {}
};

/// Thrown when an option already exists
struct OptionAlreadyAdded : public ConstructionError {
    OptionAlreadyAdded(std::string name) : ConstructionError("OptionAlreadyAdded", name, 3) {}
};

// Parsing errors

/// Anything that can error in Parse
struct ParseError : public Error {
    ParseError(std::string parent, std::string name, int exit_code=255, bool print_help=true) : Error(parent, name, exit_code, print_help) {}
};

// Not really "errors"

/// This is a successful completion on parsing, supposed to exit
struct Success : public ParseError {
    Success() : ParseError("Success", "Successfully completed, should be caught and quit", 0, false) {}
};

/// -h or --help on command line
struct CallForHelp : public ParseError {
    CallForHelp() : ParseError("CallForHelp", "This should be caught in your main function, see examples", 0) {}
};


/// Thrown when parsing an INI file and it is missing
struct FileError : public ParseError {
    FileError (std::string name) : ParseError("FileError", name, 10) {}
};

/// Thrown when conversion call back fails, such as when an int fails to coerse to a string
struct ConversionError : public ParseError {
    ConversionError(std::string name) : ParseError("ConversionError", name, 2) {}
};

/// Thrown when validation of results fails
struct ValidationError : public ParseError {
    ValidationError(std::string name) : ParseError("ValidationError", name, 2) {}
};

/// Thrown when a required option is missing
struct RequiredError : public ParseError {
    RequiredError(std::string name) : ParseError("RequiredError", name, 5) {}
};

/// Thrown when a requires option is missing
struct RequiresError : public ParseError {
    RequiresError(std::string name, std::string subname) : ParseError("RequiresError", name + " requires " + subname, 13) {}
};

/// Thrown when a exludes option is present
struct ExcludesError : public ParseError {
    ExcludesError(std::string name, std::string subname) : ParseError("ExcludesError", name + " excludes " + subname, 14) {}
};

/// Thrown when too many positionals are found
struct PositionalError : public ParseError {
    PositionalError(std::string name) : ParseError("PositionalError", name, 6) {}
};

/// This is just a safety check to verify selection and parsing match
struct HorribleError : public ParseError {
    HorribleError(std::string name) : ParseError("HorribleError", "(You should never see this error) " + name, 7) {}
};

// After parsing

/// Thrown when counting a non-existent option
struct OptionNotFound : public Error {
    OptionNotFound(std::string name) : Error("OptionNotFound", name, 4) {}
};

/// @}

}
