// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"
#include "../ExtraValidators.hpp"

#if (defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS == 1) ||                                  \
    (!defined(CLI11_DISABLE_EXTRA_VALIDATORS) || CLI11_DISABLE_EXTRA_VALIDATORS == 0)

#include "../Encoding.hpp"
#include "../Macros.hpp"
#include "../StringTools.hpp"
#include "../TypeTools.hpp"

// [CLI11:public_includes:set]
#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <utility>
// [CLI11:public_includes:end]

namespace CLI {
// [CLI11:extra_validators_inl_hpp:verbatim]

namespace detail {

CLI11_INLINE IPV4Validator::IPV4Validator() : Validator("IPV4") {
    func_ = [](std::string &ip_addr) {
        auto cdot = std::count(ip_addr.begin(), ip_addr.end(), '.');
        if(cdot != 3u) {
            return std::string("Invalid IPV4 address: must have 3 separators");
        }
        auto result = CLI::detail::split(ip_addr, '.');
        if(result.size() != 4) {
            return std::string("Invalid IPV4 address: must have four parts (") + ip_addr + ')';
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

#if defined(CLI11_ENABLE_EXTRA_VALIDATORS) && CLI11_ENABLE_EXTRA_VALIDATORS != 0
// new extra validators
namespace detail {

#if defined CLI11_HAS_FILESYSTEM && CLI11_HAS_FILESYSTEM > 0
CLI11_INLINE PermissionValidator::PermissionValidator(Permission permission) {
    std::filesystem::perms permission_code = std::filesystem::perms::none;
    std::string permission_name;
    switch(permission) {
    case Permission::read:
        permission_code = std::filesystem::perms::owner_read | std::filesystem::perms::group_read |
                          std::filesystem::perms::others_read;
        permission_name = "read";
        break;
    case Permission::write:
        permission_code = std::filesystem::perms::owner_write | std::filesystem::perms::group_write |
                          std::filesystem::perms::others_write;
        permission_name = "write";
        break;
    case Permission::exec:
        permission_code = std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec |
                          std::filesystem::perms::others_exec;
        permission_name = "exec";
        break;
    case Permission::none:
    default:
        permission_code = std::filesystem::perms::none;
        break;
    }
    func_ = [permission_code](std::string &path) {
        std::error_code ec;
        auto p = std::filesystem::path(path);
        if(!std::filesystem::exists(p, ec)) {
            return std::string("Path does not exist: ") + path;
        }
        if(ec) {
            return std::string("Error checking path: ") + ec.message();  // LCOV_EXCL_LINE
        }
        if(permission_code == std::filesystem::perms::none) {
            return std::string{};
        }
        auto perms = std::filesystem::status(p, ec).permissions();
        if(ec) {
            return std::string("Error checking path status: ") + ec.message();  // LCOV_EXCL_LINE
        }
        if((perms & permission_code) == std::filesystem::perms::none) {
            return std::string("Path does not have required permissions: ") + path;
        }
        return std::string{};
    };
    description("Path with " + permission_name + " permission");
}
#endif

}  // namespace detail
#endif
   // [CLI11:extra_validators_inl_hpp:end]
}  // namespace CLI

#endif
