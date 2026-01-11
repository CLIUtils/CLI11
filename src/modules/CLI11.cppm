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
        using CLI::FailureMessage::help;
        using CLI::FailureMessage::simple;
    }  // namespace FailureMessage

    using CLI::App;
    using CLI::App_p;
    using CLI::AppFormatMode;
    using CLI::ArgumentMismatch;
    using CLI::AsNumberWithUnit;
    using CLI::AsSizeValue;
    using CLI::BadNameString;
    using CLI::Bound;
    using CLI::callback_t;
    using CLI::CallbackPriority;
    using CLI::CallForAllHelp;
    using CLI::CallForHelp;
    using CLI::CallForVersion;
    using CLI::CheckedTransformer;
    using CLI::Config;
    using CLI::config_extras_mode;
    using CLI::ConfigBase;
    using CLI::ConfigError;
    using CLI::ConfigExtrasMode;
    using CLI::ConfigINI;
    using CLI::ConfigItem;
    using CLI::ConfigTOML;
    using CLI::ConstructionError;
    using CLI::ConversionError;
    using CLI::deprecate_option;
    using CLI::Error;
    using CLI::ExcludesError;
    using CLI::ExitCodes;
    using CLI::ExtrasError;
    using CLI::ExtrasMode;
    using CLI::FileError;
    using CLI::Formatter;
    using CLI::FormatterBase;
    using CLI::FormatterLambda;
    using CLI::HorribleError;
    using CLI::ignore_case;
    using CLI::ignore_space;
    using CLI::ignore_underscore;
    using CLI::IncorrectConstruction;
    using CLI::InvalidError;
    using CLI::IsMember;
    using CLI::MultiOptionPolicy;
    using CLI::narrow;
    using CLI::Option;
    using CLI::Option_group;
    using CLI::Option_p;
    using CLI::OptionAlreadyAdded;
    using CLI::OptionBase;
    using CLI::OptionDefaults;
    using CLI::OptionNotFound;
    using CLI::ParseError;
    using CLI::PrefixCommandMode;
    using CLI::RequiredError;
    using CLI::RequiresError;
    using CLI::results_t;
    using CLI::retire_option;
    using CLI::RuntimeError;
    using CLI::Success;
    using CLI::to_path;
    using CLI::Transformer;
    using CLI::TransformPairs;
    using CLI::TriggerOff;
    using CLI::TriggerOn;
    using CLI::TypeValidator;
    using CLI::ValidationError;
    using CLI::Validator_p;
    using CLI::ValidIPV4;
    using CLI::widen;

    namespace enums {
    using CLI::enums::operator<<;
    }

    using CLI::operator<<;

    using CLI::AutoTimer;
    using CLI::conditional_t;
    using CLI::CustomValidator;
    using CLI::enable_if_t;
    using CLI::EscapedString;
    using CLI::ExistingDirectory;
    using CLI::ExistingFile;
    using CLI::ExistingPath;
    using CLI::FileOnDefaultPath;
    using CLI::is_bool;
    using CLI::is_copyable_ptr;
    using CLI::is_shared_ptr;
    using CLI::IsMemberType;
    using CLI::make_void;
    using CLI::NonexistentPath;
    using CLI::NonNegativeNumber;
    using CLI::PositiveNumber;
    using CLI::Range;
    using CLI::Timer;
    using CLI::Validator;
    using CLI::void_t;
}  // namespace CLI

export {
    using ::operator<<;
}
