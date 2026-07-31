if(CMAKE_VERSION VERSION_LESS 3.28)
  message(FATAL_ERROR "CLI11_MODULES requires CMake 3.28 or later")
endif()

add_library(CLI11_Module)

target_sources(CLI11_Module PUBLIC FILE_SET CXX_MODULES FILES CLI11.cppm)

target_compile_features(CLI11_Module PUBLIC cxx_std_20)

target_include_directories(CLI11_Module PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                               $<INSTALL_INTERFACE:include>)

add_library(CLI11::Module ALIAS CLI11_Module)

# Installation
install(
  TARGETS CLI11_Module
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
          FILE_SET CXX_MODULES
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CLI11/src/modules)
