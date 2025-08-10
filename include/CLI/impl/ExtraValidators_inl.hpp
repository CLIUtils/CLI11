// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)

// IWYU pragma: private, include "CLI/CLI.hpp"
#include "../ExtraValidators.hpp"

#include "../Encoding.hpp"
#include "../Macros.hpp"
#include "../StringTools.hpp"
#include "../TypeTools.hpp"

// [CLI11:public_includes:set]
#include <map>
#include <string>
#include <utility>
// [CLI11:public_includes:end]

namespace CLI {
// [CLI11:extra_validators_inl_hpp:verbatim]

namespace detail {
CLI11_INLINE IPV4Validator::IPV4Validator() : Validator("IPV4") {
    func_ = [](std::string &ip_addr) {
        auto result = CLI::detail::split(ip_addr, '.');
        if(result.size() != 4) {
            return std::string("Invalid IPV4 address must have four parts (") + ip_addr + ')';
        }
        int num = 0;
        for(const auto &var : result) {
            using CLI::detail::lexical_cast;
            bool retval = lexical_cast(var, num);
            if(!retval) {
                return std::string("Failed parsing number (") + var + ')';
            }
            if(num < 0 || num > 255) {
                return std::string("Each IP number must be between 0 and 255 ") + var;
            }
        }
        return std::string{};
    };
}

}  // namespace detail

CLI11_INLINE AsSizeValue::AsSizeValue(bool kb_is_1000) : AsNumberWithUnit(get_mapping(kb_is_1000)) {
    if(kb_is_1000) {
        description("SIZE [b, kb(=1000b), kib(=1024b), ...]");
    } else {
        description("SIZE [b, kb(=1024b), ...]");
    }
}

CLI11_INLINE std::map<std::string, AsSizeValue::result_t> AsSizeValue::init_mapping(bool kb_is_1000) {
    std::map<std::string, result_t> m;
    result_t k_factor = kb_is_1000 ? 1000 : 1024;
    result_t ki_factor = 1024;
    result_t k = 1;
    result_t ki = 1;
    m["b"] = 1;
    for(std::string p : {"k", "m", "g", "t", "p", "e"}) {
        k *= k_factor;
        ki *= ki_factor;
        m[p] = k;
        m[p + "b"] = k;
        m[p + "i"] = ki;
        m[p + "ib"] = ki;
    }
    return m;
}

CLI11_INLINE std::map<std::string, AsSizeValue::result_t> AsSizeValue::get_mapping(bool kb_is_1000) {
    if(kb_is_1000) {
        static auto m = init_mapping(true);
        return m;
    }
    static auto m = init_mapping(false);
    return m;
}

namespace detail {}  // namespace detail
/// @}

// [CLI11:extra_validators_inl_hpp:end]
}  // namespace CLI

#endif
