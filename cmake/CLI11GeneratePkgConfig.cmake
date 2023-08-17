if(CLI11_PRECOMPILED)
  configure_file("cmake/CLI11.pc.in" "CLI11.pc" @ONLY)
elseif(CLI11_SINGLE_FILE)
  configure_file("cmake/CLI11single.pc.in" "CLI11.pc" @ONLY)
else()
  configure_file("cmake/CLI11precompiled.pc.in" "CLI11.pc" @ONLY)
endif()

install(FILES "${PROJECT_BINARY_DIR}/CLI11.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
