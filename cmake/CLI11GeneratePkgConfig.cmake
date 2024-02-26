configure_file("cmake/CLI11.pc.in" "CLI11.pc" @ONLY)

install(FILES "${PROJECT_BINARY_DIR}/CLI11.pc" DESTINATION "${CMAKE_INSTALL_DATADIR}/pkgconfig")
