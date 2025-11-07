#pragma once
#include <string>

namespace cliutils {

// Returns the reverse of a given string
inline std::string reverse_string(const std::string &input) {
    return std::string(input.rbegin(), input.rend());
}

}
