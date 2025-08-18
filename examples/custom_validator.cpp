// Copyright (c) 2017-2025, University of Cincinnati, developed by Henry Schreiner
// under NSF AWARD 1414736 and by the respective contributors.
// All rights reserved.
//
// SPDX-License-Identifier: BSD-3-Clause

#include <CLI/CLI.hpp>
#include <iostream>
#include <memory>
#include <string>

template <typename T> class DeltaRange : public CLI::Validator {
  public:
    T center_point;
    T delta;
    DeltaRange(const T &center, const T &range)
        : CLI::Validator(
              [this](const std::string &value) -> std::string {
                  T newValue;
                  auto result = CLI::detail::lexical_cast(value, newValue);
                  if(!(result && this->check(newValue))) {
                      return std::string("value not within range");
                  }
                  return std::string{};
              },
              "RANGE"),
          center_point(center), delta(range) {}

    CLI11_NODISCARD bool check(const T &test) const {
        return (test >= (center_point - delta)) && (test <= (center_point + delta));
    }
    CLI11_NODISCARD T center() const { return center_point; }
    CLI11_NODISCARD T range() const { return delta; }
    void center(const T &value) { center_point = value; }
    void range(const T &value) { delta = value; }
};

int main(int argc, char **argv) {
    /* this application creates custom validator which is a range center+/- range The center and range can be defined by
     * other command line options and are updated dynamically
     */
    CLI::App app("custom range validator");

    std::string value;
    auto dr = std::make_shared<DeltaRange<int>>(7, 3);
    app.add_option("--number", value, "enter value in the related range")->check(dr)->required();

    app.add_option_function<int>("--center", [&dr](int new_center) { dr->center(new_center); })->trigger_on_parse();
    app.add_option_function<int>("--range", [&dr](int new_range) { dr->range(new_range); })->trigger_on_parse();

    CLI11_PARSE(app, argc, argv);

    std::cout << "number " << value << " in range = " << dr->center() << " +/- " << dr->range() << '\n';

    return 0;
}
