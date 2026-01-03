// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

module;

#include <CLI/CLI.hpp>
#include <CLI/Timer.hpp>

export module cli11;

export namespace CLI {
    namespace FailureMessage {
        using CLI::FailureMessage::simple;
        using CLI::FailureMessage::help;
    }

    using CLI::ExtrasMode;
    using CLI::ConfigExtrasMode;
    using CLI::config_extras_mode;
    using CLI::PrefixCommandMode;
    using CLI::App;
    using CLI::App_p;
    using CLI::Option_group;
    using CLI::TriggerOn;
    using CLI::TriggerOff;
    using CLI::deprecate_option;
    using CLI::retire_option;
    using CLI::ConfigItem;
    using CLI::Config;
    using CLI::ConfigBase;
    using CLI::ConfigTOML;
    using CLI::ConfigINI;
    using CLI::narrow;
    using CLI::widen;
    using CLI::to_path;
    using CLI::ExitCodes;
    using CLI::Error;
    using CLI::ConstructionError;
    using CLI::IncorrectConstruction;
    using CLI::BadNameString;
    using CLI::OptionAlreadyAdded;
    using CLI::ParseError;
    using CLI::Success;
    using CLI::CallForHelp;
    using CLI::CallForAllHelp;
    using CLI::CallForVersion;
    using CLI::RuntimeError;
    using CLI::FileError;
    using CLI::ConversionError;
    using CLI::ValidationError;
    using CLI::RequiredError;
    using CLI::ArgumentMismatch;
    using CLI::RequiresError;
    using CLI::ExcludesError;
    using CLI::ExtrasError;
    using CLI::ConfigError;
    using CLI::InvalidError;
    using CLI::HorribleError;
    using CLI::OptionNotFound;
    using CLI::TypeValidator;
    using CLI::Bound;
    using CLI::ValidIPV4;
    using CLI::IsMember;
    using CLI::TransformPairs;
    using CLI::Transformer;
    using CLI::CheckedTransformer;
    using CLI::ignore_case;
    using CLI::ignore_underscore;
    using CLI::ignore_space;
    using CLI::AsNumberWithUnit;
    using CLI::AsSizeValue;
    using CLI::Option;
    using CLI::AppFormatMode;
    using CLI::FormatterBase;
    using CLI::FormatterLambda;
    using CLI::Formatter;
    using CLI::results_t;
    using CLI::callback_t;
    using CLI::Option_p;
    using CLI::Validator_p;
    using CLI::MultiOptionPolicy;
    using CLI::CallbackPriority;
    using CLI::OptionBase;
    using CLI::OptionDefaults;

    namespace enums {
        using CLI::enums::operator<<;
    }

    using CLI::operator<<;

    using CLI::Timer;
    using CLI::AutoTimer;
    using CLI::enable_if_t;
    using CLI::make_void;
    using CLI::void_t;
    using CLI::conditional_t;
    using CLI::is_bool;
    using CLI::is_shared_ptr;
    using CLI::is_copyable_ptr;
    using CLI::IsMemberType;
    using CLI::Validator;
    using CLI::CustomValidator;
    using CLI::ExistingFile;
    using CLI::ExistingDirectory;
    using CLI::ExistingPath;
    using CLI::NonexistentPath;
    using CLI::EscapedString;
    using CLI::FileOnDefaultPath;
    using CLI::Range;
    using CLI::NonNegativeNumber;
    using CLI::PositiveNumber;
}

export {
    using ::operator<<;
}
