# Validators

There are two forms of validators:

- `transform` validators: mutating
- `check` validators: non-mutating (recommended unless the parsed string must be
  mutated)

A transform validator comes in one form, a function with the signature
`std::string(std::string&)`. The function will take a string and return an error
message, or an empty string if input is valid. If there is an error, the
function should throw a `CLI::ValidationError` with the appropriate reason as a
message.

An example of a mutating validator:

```cpp
auto transform_validator = CLI::Validator(
        [](std::string &input) {
            if (input == "error") {
                return "error is not a valid value";
            } else if (input == "unexpected") {
                throw CLI::ValidationError{"Unexpected error"};
            }
            input = "new string";
            return "";
        }, "VALIDATOR DESCRIPTION", "Validator name");

cli_global.add_option("option")->transform(transform_validator);
```

However, `check` validators come in two forms; either a simple function with the
const version of the above signature, `std::string(const std::string &)`, or a
subclass of `struct CLI::Validator`. This structure has two members that a user
should set; one (`func_`) is the function to add to the Option (exactly matching
the above function signature, since it will become that function), and the other
is `name_`, and is the type name to set on the Option (unless empty, in which
case the typename will be left unchanged).

Validators can be combined with `&` and `|`, and they have an `operator()` so
that you can call them as if they were a function. In CLI11, const static
versions of the validators are provided so that the user does not have to call a
constructor also.

An example of a custom validator:

```cpp
struct LowerCaseValidator : public Validator {
    LowerCaseValidator() {
        name_ = "LOWER";
        func_ = [](const std::string &str) {
            if(CLI::detail::to_lower(str) != str)
                return std::string("String is not lower case");
            else
                return std::string();
        };
    }
};
const static LowerCaseValidator Lowercase;
```

If you were not interested in the extra features of Validator, you could simply
pass the lambda function above to the `->check()` method of `Option`.

The built-in validators for CLI11 are:

| Validator           | Description                                                            |
| ------------------- | ---------------------------------------------------------------------- |
| `ExistingFile`      | Check for existing file (returns error message if check fails)         |
| `ExistingDirectory` | Check for an existing directory (returns error message if check fails) |
| `ExistingPath`      | Check for an existing path                                             |
| `NonexistentPath`   | Check for an non-existing path                                         |
| `Range(min=0, max)` | Produce a range (factory). Min and max are inclusive.                  |
| `NonNegativeNumber` | Range(0,max<double>)                                                   |
| `PositiveNumber`    | Range(epsilon,max<double>)                                             |

A few built-in transformers are also available

| Transformer         | Description                                                |
| ------------------- | ---------------------------------------------------------- |
| `EscapedString`     | modify a string using defined escape characters            |
| `FileOnDefaultPath` | Modify a path if the file is a particular default location |

And, the protected members that you can set when you make your own are:

| Type                                        | Member               | Description                                                            |
| ------------------------------------------- | -------------------- | ---------------------------------------------------------------------- |
| `std::function<std::string(std::string &)>` | `func_`              | Core validation function - modifies input and returns "" if successful |
| `std::function<std::string()>`              | `desc_function`      | Optional description function (uses `description_` instead if not set) |
| `std::string`                               | `name_`              | The name for search purposes                                           |
| `int` (`-1`)                                | `application_index_` | The element this validator applies to (-1 for all)                     |
| `bool` (`true`)                             | `active_`            | This can be disabled                                                   |
| `bool` (`false`)                            | `non_modifying_`     | Specify that this is a Validator instead of a Transformer              |

## Extra Validators

Until CLI11 v3.0 these validators will be available by default. They can be
disabled at compilation time by defining CLI11_DISABLE_EXTRA_VALIDATORS to 1.
After version 3.0 they can be enabled by defining CLI11_ENABLE_EXTRA_VALIDATORS
to 1. Some of the Validators are template heavy so if they are not needed and
compilation time is a concern they can be disabled.

| Validator            | Description                                                        |
| -------------------- | ------------------------------------------------------------------ |
| `ValidIPV4`          | check for valid IPV4 address XX.XX.XX.XX                           |
| `TypeValidator<T>`   | template for checking that a value can convert to a specific type  |
| `Number`             | Check that a value can convert to a number                         |
| `IsMember`           | Check that a value is one of a set of values                       |
| `CheckedTransformer` | Values must be one of the transformed set or the result            |
| `AsNumberWithUnit`   | checks for numbers with a unit as part of a specified set of units |
| `AsSizeValue`        | As Number with Unit with support for SI prefixes                   |

| Transformer            | Description                                         |
| ---------------------- | --------------------------------------------------- |
| `Bound<T>(min=0, max)` | Force a range (factory). Min and max are inclusive. |
| `Transformer`          | Modify values in a set to the matching pair value   |

## New Extra Validators

Some additional validators can be enabled by using CLI11_ENABLE_EXTRA_VALIDATORS
to 1. These validators are disabled by default.

| Validator          | Description                                                 |
| ------------------ | ----------------------------------------------------------- |
| `ReadPermissions`  | Ensure a file or directory has permissions to read the file |
| `WritePermissions` | Ensure a file or directory has write permissions            |
| `ExecPermissions`  | Ensure a file has exec permissions                          |

## Custom Validators

CLI11 also supports the use of custom validators, this includes using the
Validator class constructor with a custom function calls or subclassing
Validator to define a new class.

### Custom Validator operation

The simplest way to make a new Validator is to mimic how many of the existing
Validators are created. Take for example the `IPV4Validator`

```cpp
class IPV4Validator : public Validator {
  public:
    IPV4Validator();
};

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
```

The `IPV4Validator` class inherits from `Validator` and creates a new
constructor. In that constructor it defines the lambda function that does the
checking. Then IPV4 can be used like any other Validator. One specific item of
note is that the class does not define any new member variables, so the class if
copyable to a Validator, only the constructor is different.

If additional members are needed, then the `check` and `transform` overloads
that use shared pointers need to be used. The other overloads pass by value so
polymorphism doesn't work. The custom_validator example shows a case like this.

```cpp
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

    CLI11_NODISCARD bool check(const T &test) const { return (test >= (center_point - delta)) && (test <= (center_point + delta)); }
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
```

The Validator defines some new operations, and in the use case the Validator is
constructed using shared_ptrs. This allows polymorphism to work and the
Validator instance to be shared across multiple options, and as in this example
adapted during the parsing and checking.

There are a few limitation in this, single instances should not be used with
both transform and check. Check modifies some flags in the Validator to prevent
value modification, so that would prevent its use as a transform. Which could be
user modified later but that would potentially allow the check to modify the
value unintentionally.
