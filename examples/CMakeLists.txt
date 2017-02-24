add_executable(try try.cpp ${CLI_headers})
target_link_libraries(try PUBLIC CLI)

add_executable(try1 try1.cpp ${CLI_headers})
target_link_libraries(try1 PUBLIC CLI)

add_executable(try2 try2.cpp ${CLI_headers})
target_link_libraries(try2 PUBLIC CLI)
