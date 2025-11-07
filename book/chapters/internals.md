# CLI11 Internals

## Callbacks

The library was designed to bind to existing variables without requiring typed
classes or inheritance. This is accomplished through lambda functions.

This looks like:

```cpp
Option* add_option(string name, T &item) {
    this->function = [&item](string value){
        return lexical_cast(value, item);
    }
}
```

Obviously, you can't access `T` after the `add_` method is over, so it stores
the string representation of the default value if it receives the special `true`
value as the final argument (not shown above).

## Parsing

Parsing follows the following procedure:

1. `_validate`: Make sure the defined options and subcommands are self
   consistent.
2. `_parse`: Main parsing routine. See below.
3. `_run_callback`: Run an App callback if present.

The parsing phase is the most interesting:

1. `_parse_single`: Run on each entry on the command line and fill the
   options/subcommands.
2. `_process`: Run the procedure listed below.
3. `_process_extra`: This throws an error if needed on extra arguments that
   didn't fit in the parse.

The `_process` procedure runs the following steps; each step is recursive and
completes all subcommands before moving to the next step. This ensures that
interactions between options and subcommand options is consistent.

```c++
CLI11_INLINE void App::_process() {
    // help takes precedence over other potential errors and config and environment shouldn't be processed if help
    // throws
    _process_callbacks(CallbackPriority::FirstPreHelp);
    _process_help_flags(CallbackPriority::First);
    _process_callbacks(CallbackPriority::First);

    std::exception_ptr config_exception;
    try {
        // the config file might generate a FileError but that should not be processed until later in the process
        // to allow for help, version and other errors to generate first.
        _process_config_file();

        // process env shouldn't throw but no reason to process it if config generated an error
        _process_env();
    } catch(const CLI::FileError &) {
        config_exception = std::current_exception();
    }
    // callbacks and requirements processing can generate exceptions which should take priority
    // over the config file error if one exists.
    _process_callbacks(CallbackPriority::PreRequirementsCheckPreHelp);
    _process_help_flags(CallbackPriority::PreRequirementsCheck);
    _process_callbacks(CallbackPriority::PreRequirementsCheck);

    _process_requirements();

    _process_callbacks(CallbackPriority::NormalPreHelp);
    _process_help_flags(CallbackPriority::Normal);
    _process_callbacks(CallbackPriority::Normal);

    if(config_exception) {
        std::rethrow_exception(config_exception);
    }

    _process_callbacks(CallbackPriority::LastPreHelp);
    _process_help_flags(CallbackPriority::Last);
    _process_callbacks(CallbackPriority::Last);
}

```

Option callbacks can be executed at many different stages depending on the
priority specified. The default is `Normal` so they will execute after
processing requirements. The default for help and version flags is to execute
`First`. Both can be changed to execute in different steps of the process.

## Exceptions

The library immediately returns a C++ exception when it detects a problem, such
as an incorrect construction or a malformed command line. Errors from config
processing are delayed until after other processing, to give priority to any
help or version flags, or other types of callback errors.
