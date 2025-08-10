// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#define CLI11_DISABLE_EXTRA_VALIDATORS 1
#pragma once
#if !defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS==0
// IWYU pragma: private, include "CLI/CLI.hpp"

#include "Error.hpp"
#include "Macros.hpp"
#include "StringTools.hpp"
#include "Validators.hpp"

// [CLI11:public_includes:set]
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
// [CLI11:public_includes:end]


namespace CLI {
// [CLI11:extra_validators_hpp:verbatim]
// The implementation of the extra validators is using the Validator class;
// the user is only expected to use the const (static) versions (since there's no setup).
// Therefore, this is in detail.
namespace detail {

/// Validate the given string is a legal ipv4 address
class IPV4Validator : public Validator {
  public:
    IPV4Validator();
};


}  // namespace detail

/// Check for an IP4 address
const detail::IPV4Validator ValidIPV4;


/// Multiply a number by a factor using given mapping.
/// Can be used to write transforms for SIZE or DURATION inputs.
///
/// Example:
///   With mapping = `{"b"->1, "kb"->1024, "mb"->1024*1024}`
///   one can recognize inputs like "100", "12kb", "100 MB",
///   that will be automatically transformed to 100, 14448, 104857600.
///
/// Output number type matches the type in the provided mapping.
/// Therefore, if it is required to interpret real inputs like "0.42 s",
/// the mapping should be of a type <string, float> or <string, double>.
class AsNumberWithUnit : public Validator {
  public:
    /// Adjust AsNumberWithUnit behavior.
    /// CASE_SENSITIVE/CASE_INSENSITIVE controls how units are matched.
    /// UNIT_OPTIONAL/UNIT_REQUIRED throws ValidationError
    ///   if UNIT_REQUIRED is set and unit literal is not found.
    enum Options : std::uint8_t {
        CASE_SENSITIVE = 0,
        CASE_INSENSITIVE = 1,
        UNIT_OPTIONAL = 0,
        UNIT_REQUIRED = 2,
        DEFAULT = CASE_INSENSITIVE | UNIT_OPTIONAL
    };

    template <typename Number>
    explicit AsNumberWithUnit(std::map<std::string, Number> mapping,
                              Options opts = DEFAULT,
                              const std::string &unit_name = "UNIT") {
        description(generate_description<Number>(unit_name, opts));
        validate_mapping(mapping, opts);

        // transform function
        func_ = [mapping, opts](std::string &input) -> std::string {
            Number num{};

            detail::rtrim(input);
            if(input.empty()) {
                throw ValidationError("Input is empty");
            }

            // Find split position between number and prefix
            auto unit_begin = input.end();
            while(unit_begin > input.begin() && std::isalpha(*(unit_begin - 1), std::locale())) {
                --unit_begin;
            }

            std::string unit{unit_begin, input.end()};
            input.resize(static_cast<std::size_t>(std::distance(input.begin(), unit_begin)));
            detail::trim(input);

            if(opts & UNIT_REQUIRED && unit.empty()) {
                throw ValidationError("Missing mandatory unit");
            }
            if(opts & CASE_INSENSITIVE) {
                unit = detail::to_lower(unit);
            }
            if(unit.empty()) {
                using CLI::detail::lexical_cast;
                if(!lexical_cast(input, num)) {
                    throw ValidationError(std::string("Value ") + input + " could not be converted to " +
                                          detail::type_name<Number>());
                }
                // No need to modify input if no unit passed
                return {};
            }

            // find corresponding factor
            auto it = mapping.find(unit);
            if(it == mapping.end()) {
                throw ValidationError(unit +
                                      " unit not recognized. "
                                      "Allowed values: " +
                                      detail::generate_map(mapping, true));
            }

            if(!input.empty()) {
                using CLI::detail::lexical_cast;
                bool converted = lexical_cast(input, num);
                if(!converted) {
                    throw ValidationError(std::string("Value ") + input + " could not be converted to " +
                                          detail::type_name<Number>());
                }
                // perform safe multiplication
                bool ok = detail::checked_multiply(num, it->second);
                if(!ok) {
                    throw ValidationError(detail::to_string(num) + " multiplied by " + unit +
                                          " factor would cause number overflow. Use smaller value.");
                }
            } else {
                num = static_cast<Number>(it->second);
            }

            input = detail::to_string(num);

            return {};
        };
    }

  private:
    /// Check that mapping contains valid units.
    /// Update mapping for CASE_INSENSITIVE mode.
    template <typename Number> static void validate_mapping(std::map<std::string, Number> &mapping, Options opts) {
        for(auto &kv : mapping) {
            if(kv.first.empty()) {
                throw ValidationError("Unit must not be empty.");
            }
            if(!detail::isalpha(kv.first)) {
                throw ValidationError("Unit must contain only letters.");
            }
        }

        // make all units lowercase if CASE_INSENSITIVE
        if(opts & CASE_INSENSITIVE) {
            std::map<std::string, Number> lower_mapping;
            for(auto &kv : mapping) {
                auto s = detail::to_lower(kv.first);
                if(lower_mapping.count(s)) {
                    throw ValidationError(std::string("Several matching lowercase unit representations are found: ") +
                                          s);
                }
                lower_mapping[detail::to_lower(kv.first)] = kv.second;
            }
            mapping = std::move(lower_mapping);
        }
    }

    /// Generate description like this: NUMBER [UNIT]
    template <typename Number> static std::string generate_description(const std::string &name, Options opts) {
        std::stringstream out;
        out << detail::type_name<Number>() << ' ';
        if(opts & UNIT_REQUIRED) {
            out << name;
        } else {
            out << '[' << name << ']';
        }
        return out.str();
    }
};

inline AsNumberWithUnit::Options operator|(const AsNumberWithUnit::Options &a, const AsNumberWithUnit::Options &b) {
    return static_cast<AsNumberWithUnit::Options>(static_cast<int>(a) | static_cast<int>(b));
}

/// Converts a human-readable size string (with unit literal) to uin64_t size.
/// Example:
///   "100" => 100
///   "1 b" => 100
///   "10Kb" => 10240 // you can configure this to be interpreted as kilobyte (*1000) or kibibyte (*1024)
///   "10 KB" => 10240
///   "10 kb" => 10240
///   "10 kib" => 10240 // *i, *ib are always interpreted as *bibyte (*1024)
///   "10kb" => 10240
///   "2 MB" => 2097152
///   "2 EiB" => 2^61 // Units up to exibyte are supported
class AsSizeValue : public AsNumberWithUnit {
  public:
    using result_t = std::uint64_t;

    /// If kb_is_1000 is true,
    /// interpret 'kb', 'k' as 1000 and 'kib', 'ki' as 1024
    /// (same applies to higher order units as well).
    /// Otherwise, interpret all literals as factors of 1024.
    /// The first option is formally correct, but
    /// the second interpretation is more wide-spread
    /// (see https://en.wikipedia.org/wiki/Binary_prefix).
    explicit AsSizeValue(bool kb_is_1000);

  private:
    /// Get <size unit, factor> mapping
    static std::map<std::string, result_t> init_mapping(bool kb_is_1000);

    /// Cache calculated mapping
    static std::map<std::string, result_t> get_mapping(bool kb_is_1000);
};


// [CLI11:extra_validators_hpp:end]
}  // namespace CLI

#ifndef CLI11_COMPILE
#include "impl/ExtraValidators_inl.hpp"  // IWYU pragma: export
#endif

#endif
