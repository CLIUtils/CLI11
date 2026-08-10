// Copyright (c) 2017-2026, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

// IWYU pragma: private, include "CLI/CLI.hpp"

// This include is only needed for IDEs to discover symbols
#include "../TypeTools.hpp"

#include "../StringTools.hpp"

// [CLI11:public_includes:set]
#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
// [CLI11:public_includes:end]

namespace CLI {
// [CLI11:type_tools_inl_hpp:verbatim]

namespace detail {

CLI11_INLINE std::int64_t to_flag_value(std::string val) noexcept {
    static const std::string trueString("true");
    static const std::string falseString("false");
    if(val == trueString) {
        return 1;
    }
    if(val == falseString) {
        return -1;
    }
    val = detail::to_lower(std::move(val));
    std::int64_t ret = 0;
    if(val.size() == 1) {
        if(val[0] >= '1' && val[0] <= '9') {
            return (static_cast<std::int64_t>(val[0]) - '0');
        }
        switch(val[0]) {
        case '0':
        case 'f':
        case 'n':
        case '-':
            ret = -1;
            break;
        case 't':
        case 'y':
        case '+':
            ret = 1;
            break;
        default:
            errno = EINVAL;
            return -1;
        }
        return ret;
    }
    if(val == trueString || val == "on" || val == "yes" || val == "enable") {
        ret = 1;
    } else if(val == falseString || val == "off" || val == "no" || val == "disable") {
        ret = -1;
    } else {
        char *loc_ptr{nullptr};
        ret = std::strtoll(val.c_str(), &loc_ptr, 0);
        if(loc_ptr != (val.c_str() + val.size()) && errno == 0) {
            errno = EINVAL;
        }
    }
    return ret;
}

CLI11_INLINE std::string sum_string_vector(const std::vector<std::string> &values) {
    // First try a pure integer sum so that large integral totals (>= 1e16, or beyond the 2^53 mantissa of
    // double) are preserved exactly and rendered as a plain integer rather than scientific notation
    std::int64_t ival{0};
    bool int_sum{true};
    for(const auto &arg : values) {
        std::int64_t tv{0};
        if(!integral_conversion(arg, tv)) {
            int_sum = false;
            break;
        }
        // detect signed overflow before performing the addition since signed overflow is undefined behavior
        if((tv > 0 && ival > (std::numeric_limits<std::int64_t>::max)() - tv) ||
           (tv < 0 && ival < (std::numeric_limits<std::int64_t>::min)() - tv)) {
            int_sum = false;
            break;
        }
        ival += tv;
    }
    if(int_sum) {
        return std::to_string(ival);
    }

    double val{0.0};
    bool fail{false};
    std::string output;
    for(const auto &arg : values) {
        double tv{0.0};
        auto comp = lexical_cast(arg, tv);
        if(!comp) {
            errno = 0;
            auto fv = detail::to_flag_value(arg);
            fail = (errno != 0);
            if(fail) {
                break;
            }
            tv = static_cast<double>(fv);
        }
        val += tv;
    }
    if(fail) {
        for(const auto &arg : values) {
            output.append(arg);
        }
    } else {
        std::ostringstream out;
        out.precision(16);
        out << val;
        output = out.str();
    }
    return output;
}

}  // namespace detail
// [CLI11:type_tools_inl_hpp:end]
}  // namespace CLI
