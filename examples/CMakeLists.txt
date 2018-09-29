function(add_cli_exe T)
    add_executable(${T} ${ARGN} ${CLI11_headers})
    target_link_libraries(${T} PUBLIC CLI11)
    set_target_properties(
         ${T} PROPERTIES
         FOLDER "Examples"
         )

    if(CLANG_TIDY_EXE)
    set_target_properties(
        ${T} PROPERTIES
        CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
        )
    endif()
endfunction()

option(CLI11_EXAMPLE_JSON OFF)
if(CLI11_EXAMPLE_JSON)
    if(NOT EXISTS "${CLI11_SOURCE_DIR}/extern/json/single_include/nlohmann/json.hpp")
        message(ERROR "You are missing the json package for CLI11_EXAMPLE_JSON. Please update your submodules (git submodule update --init)")
    endif()
    if(CMAKE_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
        message(WARNING "The json example requires GCC 4.9+ (requirement on json library)")
    endif()
    add_cli_exe(json json.cpp)
    target_include_directories(json PUBLIC SYSTEM "${CLI11_SOURCE_DIR}/extern/json/single_include")

    add_test(NAME json_config_out COMMAND json --item 2)
    set_property(TEST json_config_out PROPERTY PASS_REGULAR_EXPRESSION
        "{"
        "\"item\": \"2\""
        "\"simple\": false"
        "}")

    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/input.json" [=[{"item":3,"simple":false}]=])
    add_test(NAME json_config_in COMMAND json --config "${CMAKE_CURRENT_BINARY_DIR}/input.json")
    set_property(TEST json_config_in PROPERTY PASS_REGULAR_EXPRESSION
        "{"
        "\"item\": \"3\""
        "\"simple\": false"
        "}")
endif()

add_cli_exe(simple simple.cpp)
add_test(NAME simple_basic COMMAND simple)
add_test(NAME simple_all COMMAND simple -f filename.txt -c 12 --flag --flag -d 1.2)
set_property(TEST simple_all PROPERTY PASS_REGULAR_EXPRESSION
    "Working on file: filename.txt, direct count: 1, opt count: 1"
    "Working on count: 12, direct count: 1, opt count: 1"
    "Received flag: 2 (2) times"
    "Some value: 1.2")


add_cli_exe(subcommands subcommands.cpp)
add_test(NAME subcommands_none COMMAND subcommands)
set_property(TEST subcommands_none PROPERTY
    PASS_REGULAR_EXPRESSION "A subcommand is required")
add_test(NAME subcommands_all COMMAND subcommands --random start --file name stop --count)
set_property(TEST subcommands_all PROPERTY PASS_REGULAR_EXPRESSION
    "Working on --file from start: name"
    "Working on --count from stop: 1, direct count: 1"
    "Count of --random flag: 1"
    "Subcommand: start"
    "Subcommand: stop")

add_cli_exe(validators validators.cpp)
add_test(NAME validators_help COMMAND validators --help)
set_property(TEST validators_help PROPERTY PASS_REGULAR_EXPRESSION
    "  -f,--file FILE              File name"
    "  -v,--value INT in [3 - 6]   Value in range")
add_test(NAME validators_file COMMAND validators --file nonex.xxx)
set_property(TEST validators_file PROPERTY PASS_REGULAR_EXPRESSION
    "--file: File does not exist: nonex.xxx"
    "Run with --help for more information.")
add_test(NAME validators_plain COMMAND validators --value 9)
set_property(TEST validators_plain PROPERTY PASS_REGULAR_EXPRESSION
    "--value: Value 9 not in range 3 to 6"
    "Run with --help for more information.")

add_cli_exe(groups groups.cpp)
add_test(NAME groups_none COMMAND groups)
set_property(TEST groups_none PROPERTY PASS_REGULAR_EXPRESSION
    "This is a timer:"
    "--file is required"
    "Run with --help for more information.")
add_test(NAME groups_all COMMAND groups --file this --count --count -d 1.2)
set_property(TEST groups_all PROPERTY PASS_REGULAR_EXPRESSION
    "This is a timer:"
    "Working on file: this, direct count: 1, opt count: 1"
    "Working on count: 2, direct count: 2, opt count: 2"
    "Some value: 1.2")

add_cli_exe(inter_argument_order inter_argument_order.cpp)
add_test(NAME inter_argument_order COMMAND inter_argument_order --foo 1 2 3 --x --bar 4 5 6 --z --foo 7 8)
set_property(TEST inter_argument_order PROPERTY PASS_REGULAR_EXPRESSION
    [=[foo : 1
foo : 2
foo : 3
bar : 4
bar : 5
bar : 6
foo : 7
foo : 8]=])

add_cli_exe(prefix_command prefix_command.cpp)
add_test(NAME prefix_command COMMAND prefix_command -v 3 2 1 -- other one two 3)
set_property(TEST prefix_command PROPERTY PASS_REGULAR_EXPRESSION
    "Prefix: 3 : 2 : 1"
    "Remaining commands: other one two 3")

add_cli_exe(enum enum.cpp)
add_test(NAME enum_pass COMMAND enum -l 1)
add_test(NAME enum_fail COMMAND enum -l 4)
set_property(TEST enum_fail PROPERTY PASS_REGULAR_EXPRESSION
    "Could not convert: --level = 4")

add_cli_exe(modhelp modhelp.cpp)
add_test(NAME modhelp COMMAND modhelp -a test -h)
set_property(TEST modhelp PROPERTY PASS_REGULAR_EXPRESSION
    "Option -a string in help: test")

add_subdirectory(subcom_in_files)
add_test(NAME subcom_in_files COMMAND subcommand_main subcommand_a -f this.txt --with-foo)
set_property(TEST subcom_in_files PROPERTY PASS_REGULAR_EXPRESSION
    "Working on file: this\.txt"
    "Using foo!")

add_cli_exe(formatter formatter.cpp)

add_cli_exe(nested nested.cpp)

add_cli_exe(subcom_help subcom_help.cpp)
add_test(NAME subcom_help_normal COMMAND subcom_help sub --help)
add_test(NAME subcom_help_reversed COMMAND subcom_help --help sub)
