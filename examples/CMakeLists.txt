function(add_cli_exe T)
    add_executable(${T} ${ARGN} ${CLI_headers})
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

add_cli_exe(simple simple.cpp)
add_cli_exe(subcommands subcommands.cpp)
add_cli_exe(groups groups.cpp)
add_cli_exe(inter_argument_order inter_argument_order.cpp)
add_cli_exe(prefix_command prefix_command.cpp)
add_cli_exe(enum enum.cpp)
add_cli_exe(modhelp modhelp.cpp)

add_subdirectory(subcom_in_files)
