function(add_cli_exe T)
    add_executable(${T} ${ARGN} ${CLI_headers})
    target_link_libraries(${T} PUBLIC CLI11)

    if(CLANG_TIDY_EXE)
    set_target_properties(
         ${T} PROPERTIES
            CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
        )
    endif()
endfunction()

add_cli_exe(try try.cpp)
add_cli_exe(try1 try1.cpp)
add_cli_exe(try2 try2.cpp)
