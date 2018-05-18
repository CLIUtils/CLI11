#pragma once

// Distributed under the 3-Clause BSD License.  See accompanying
// file LICENSE or https://github.com/CLIUtils/CLI11 for details.

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

// CLI library includes
#include "CLI/StringTools.hpp"

namespace CLI {

// Use one of these on all error classes.
// These are temporary and are undef'd at the end of this file.
#define CLI11_ERROR_DEF(parent, name)                                                                                  \
  protected:                                                                                                           \
    name(std::string name, std::string msg, int exit_code) : parent(std::move(name), std::move(msg), exit_code) {}     \
    name(std::string name, std::string msg, ExitCodes exit_code)                                                       \
        : parent(std::move(name), std::move(msg), exit_code) {}                                                        \
                                                                                                                       \
  public:                                                                                                              \
    name(std::string msg, ExitCodes exit_code) : parent(#name, std::move(msg), exit_code) {}                           \
    name(std::string msg, int exit_code) : parent(#name, std::move(msg), exit_code) {}

// This is added after the one above if a class is used directly and builds its own message
#define CLI11_ERROR_SIMPLE(name)                                                                                       \
    explicit name(std::string msg) : name(#name, msg, ExitCodes::name) {}

/// These codes are part of every error in CLI. They can be obtained from e using e.exit_code or as a quick shortcut,
/// int values from e.get_error_code().
enum class ExitCodes {
    Success = 0,
    IncorrectConstruction = 100,
    BadNameString,
    OptionAlreadyAdded,
    FileError,
    ConversionError,
    ValidationError,
    RequiredError,
    RequiresError,
    ExcludesError,
    ExtrasError,
    ConfigError,
    InvalidError,
    HorribleError,
    OptionNotFound,
    ArgumentMismatch,
    BaseClass = 127
};

// Error definitions

/// @defgroup error_group Errors
/// @brief Errors thrown by CLI11
///
/// These are the errors that can be thrown. Some of them, like CLI::Success, are not really errors.
/// @{

/// All errors derive from this one
class Error : public std::runtime_error {
    int exit_code;
    std::string name{"Error"};

  public:
    int get_exit_code() const { return exit_code; }

    std::string get_name() const { return name; }

    Error(std::string name, std::string msg, int exit_code = static_cast<int>(ExitCodes::BaseClass))
        : runtime_error(msg), exit_code(exit_code), name(std::move(name)) {}

    Error(std::string name, std::string msg, ExitCodes exit_code) : Error(name, msg, static_cast<int>(exit_code)) {}
};

// Note: Using Error::Error constructors does not work on GCC 4.7

/// Construction errors (not in parsing)
class ConstructionError : public Error {
    CLI11_ERROR_DEF(Error, ConstructionError)
};

/// Thrown when an option is set to conflicting values (non-vector and multi args, for example)
class IncorrectConstruction : public ConstructionError {
    CLI11_ERROR_DEF(ConstructionError, IncorrectConstruction)
    CLI11_ERROR_SIMPLE(IncorrectConstruction)
    static IncorrectConstruction PositionalFlag(std::string name) {
        return IncorrectConstruction(name + ": Flags cannot be positional");
    }
    static IncorrectConstruction Set0Opt(std::string name) {
        return IncorrectConstruction(name + ": Cannot set 0 expected, use a flag instead");
    }
    static IncorrectConstruction SetFlag(std::string name) {
        return IncorrectConstruction(name + ": Cannot set an expected number for flags");
    }
    static IncorrectConstruction ChangeNotVector(std::string name) {
        return IncorrectConstruction(name + ": You can only change the expected arguments for vectors");
    }
    static IncorrectConstruction AfterMultiOpt(std::string name) {
        return IncorrectConstruction(
            name + ": You can't change expected arguments after you've changed the multi option policy!");
    }
    static IncorrectConstruction MissingOption(std::string name) {
        return IncorrectConstruction("Option " + name + " is not defined");
    }
    static IncorrectConstruction MultiOptionPolicy(std::string name) {
        return IncorrectConstruction(name + ": multi_option_policy only works for flags and exact value options");
    }
};

/// Thrown on construction of a bad name
class BadNameString : public ConstructionError {
    CLI11_ERROR_DEF(ConstructionError, BadNameString)
    CLI11_ERROR_SIMPLE(BadNameString)
    static BadNameString OneCharName(std::string name) { return BadNameString("Invalid one char name: " + name); }
    static BadNameString BadLongName(std::string name) { return BadNameString("Bad long name: " + name); }
    static BadNameString DashesOnly(std::string name) {
        return BadNameString("Must have a name, not just dashes: " + name);
    }
    static BadNameString MultiPositionalNames(std::string name) {
        return BadNameString("Only one positional name allowed, remove: " + name);
    }
};

/// Thrown when an option already exists
class OptionAlreadyAdded : public ConstructionError {
    CLI11_ERROR_DEF(ConstructionError, OptionAlreadyAdded)
    explicit OptionAlreadyAdded(std::string name)
        : OptionAlreadyAdded(name + " is already added", ExitCodes::OptionAlreadyAdded) {}
    static OptionAlreadyAdded Requires(std::string name, std::string other) {
        return OptionAlreadyAdded(name + " requires " + other, ExitCodes::OptionAlreadyAdded);
    }
    static OptionAlreadyAdded Excludes(std::string name, std::string other) {
        return OptionAlreadyAdded(name + " excludes " + other, ExitCodes::OptionAlreadyAdded);
    }
};

// Parsing errors

/// Anything that can error in Parse
class ParseError : public Error {
    CLI11_ERROR_DEF(Error, ParseError)
};

// Not really "errors"

/// This is a successful completion on parsing, supposed to exit
class Success : public ParseError {
    CLI11_ERROR_DEF(ParseError, Success)
    Success() : Success("Successfully completed, should be caught and quit", ExitCodes::Success) {}
};

/// -h or --help on command line
class CallForHelp : public ParseError {
    CLI11_ERROR_DEF(ParseError, CallForHelp)
    CallForHelp() : CallForHelp("This should be caught in your main function, see examples", ExitCodes::Success) {}
};

/// Usually somethign like --help-all on command line
class CallForAllHelp : public ParseError {
    CLI11_ERROR_DEF(ParseError, CallForAllHelp)
    CallForAllHelp()
        : CallForAllHelp("This should be caught in your main function, see examples", ExitCodes::Success) {}
};

/// Does not output a diagnostic in CLI11_PARSE, but allows to return from main() with a specific error code.
class RuntimeError : public ParseError {
    CLI11_ERROR_DEF(ParseError, RuntimeError)
    explicit RuntimeError(int exit_code = 1) : RuntimeError("Runtime error", exit_code) {}
};

/// Thrown when parsing an INI file and it is missing
class FileError : public ParseError {
    CLI11_ERROR_DEF(ParseError, FileError)
    CLI11_ERROR_SIMPLE(FileError)
    static FileError Missing(std::string name) { return FileError(name + " was not readable (missing?)"); }
};

/// Thrown when conversion call back fails, such as when an int fails to coerce to a string
class ConversionError : public ParseError {
    CLI11_ERROR_DEF(ParseError, ConversionError)
    CLI11_ERROR_SIMPLE(ConversionError)
    ConversionError(std::string member, std::string name)
        : ConversionError("The value " + member + " is not an allowed value for " + name) {}
    ConversionError(std::string name, std::vector<std::string> results)
        : ConversionError("Could not convert: " + name + " = " + detail::join(results)) {}
    static ConversionError TooManyInputsFlag(std::string name) {
        return ConversionError(name + ": too many inputs for a flag");
    }
    static ConversionError TrueFalse(std::string name) {
        return ConversionError(name + ": Should be true/false or a number");
    }
};

/// Thrown when validation of results fails
class ValidationError : public ParseError {
    CLI11_ERROR_DEF(ParseError, ValidationError)
    CLI11_ERROR_SIMPLE(ValidationError)
    explicit ValidationError(std::string name, std::string msg) : ValidationError(name + ": " + msg) {}
};

/// Thrown when a required option is missing
class RequiredError : public ParseError {
    CLI11_ERROR_DEF(ParseError, RequiredError)
    explicit RequiredError(std::string name) : RequiredError(name + " is required", ExitCodes::RequiredError) {}
    static RequiredError Subcommand(size_t min_subcom) {
        if(min_subcom == 1)
            return RequiredError("A subcommand");
        else
            return RequiredError("Requires at least " + std::to_string(min_subcom) + " subcommands",
                                 ExitCodes::RequiredError);
    }
};

/// Thrown when the wrong number of arguments has been received
class ArgumentMismatch : public ParseError {
    CLI11_ERROR_DEF(ParseError, ArgumentMismatch)
    CLI11_ERROR_SIMPLE(ArgumentMismatch)
    ArgumentMismatch(std::string name, int expected, size_t recieved)
        : ArgumentMismatch(expected > 0 ? ("Expected exactly " + std::to_string(expected) + " arguments to " + name +
                                           ", got " + std::to_string(recieved))
                                        : ("Expected at least " + std::to_string(-expected) + " arguments to " + name +
                                           ", got " + std::to_string(recieved)),
                           ExitCodes::ArgumentMismatch) {}

    static ArgumentMismatch AtLeast(std::string name, int num) {
        return ArgumentMismatch(name + ": At least " + std::to_string(num) + " required");
    }
    static ArgumentMismatch TypedAtLeast(std::string name, int num, std::string type) {
        return ArgumentMismatch(name + ": " + std::to_string(num) + " required " + type + " missing");
    }
};

/// Thrown when a requires option is missing
class RequiresError : public ParseError {
    CLI11_ERROR_DEF(ParseError, RequiresError)
    RequiresError(std::string curname, std::string subname)
        : RequiresError(curname + " requires " + subname, ExitCodes::RequiresError) {}
};

/// Thrown when an excludes option is present
class ExcludesError : public ParseError {
    CLI11_ERROR_DEF(ParseError, ExcludesError)
    ExcludesError(std::string curname, std::string subname)
        : ExcludesError(curname + " excludes " + subname, ExitCodes::ExcludesError) {}
};

/// Thrown when too many positionals or options are found
class ExtrasError : public ParseError {
    CLI11_ERROR_DEF(ParseError, ExtrasError)
    explicit ExtrasError(std::vector<std::string> args)
        : ExtrasError((args.size() > 1 ? "The following arguments were not expected: "
                                       : "The following argument was not expected: ") +
                          detail::rjoin(args, " "),
                      ExitCodes::ExtrasError) {}
};

/// Thrown when extra values are found in an INI file
class ConfigError : public ParseError {
    CLI11_ERROR_DEF(ParseError, ConfigError)
    CLI11_ERROR_SIMPLE(ConfigError)
    static ConfigError Extras(std::string item) { return ConfigError("INI was not able to parse " + item); }
    static ConfigError NotConfigurable(std::string item) {
        return ConfigError(item + ": This option is not allowed in a configuration file");
    }
};

/// Thrown when validation fails before parsing
class InvalidError : public ParseError {
    CLI11_ERROR_DEF(ParseError, InvalidError)
    explicit InvalidError(std::string name)
        : InvalidError(name + ": Too many positional arguments with unlimited expected args", ExitCodes::InvalidError) {
    }
};

/// This is just a safety check to verify selection and parsing match - you should not ever see it
/// Strings are directly added to this error, but again, it should never be seen.
class HorribleError : public ParseError {
    CLI11_ERROR_DEF(ParseError, HorribleError)
    CLI11_ERROR_SIMPLE(HorribleError)
};

// After parsing

/// Thrown when counting a non-existent option
class OptionNotFound : public Error {
    CLI11_ERROR_DEF(Error, OptionNotFound)
    explicit OptionNotFound(std::string name) : OptionNotFound(name + " not found", ExitCodes::OptionNotFound) {}
};

#undef CLI11_ERROR_DEF
#undef CLI11_ERROR_SIMPLE

/// @}

} // namespace CLI
