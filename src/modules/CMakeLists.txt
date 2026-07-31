if(CMAKE_VERSION VERSION_LESS 3.28)
  message(FATAL_ERROR "CLI11_MODULES requires CMake 3.28 or later")
endif()

add_library(CLI11_Module)

target_sources(CLI11_Module PUBLIC FILE_SET CXX_MODULES FILES CLI11.cppm)

target_compile_features(CLI11_Module PUBLIC cxx_std_20)

target_include_directories(CLI11_Module PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                                               $<INSTALL_INTERFACE:include>)

# Compile the module in precompiled mode: the global module fragment then holds
# only declarations, so importers do not re-optimize every function body. The
# definition is PUBLIC so importers that also include the headers agree.
target_compile_definitions(CLI11_Module PUBLIC CLI11_COMPILE)

if(CLI11_PRECOMPILED)
  # The CLI11 static library already contains the implementation.
  target_link_libraries(CLI11_Module PUBLIC CLI11)
else()
  target_sources(CLI11_Module PRIVATE ${PROJECT_SOURCE_DIR}/src/Precompile.cpp)
  if(WIN32)
    target_link_libraries(CLI11_Module PUBLIC shell32)
  endif()
endif()

add_library(CLI11::Module ALIAS CLI11_Module)
# Same name as in the installed package, for add_subdirectory consumers
add_library(CLI11::CLI11_Module ALIAS CLI11_Module)

# Installation
install(
  TARGETS CLI11_Module
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
          FILE_SET CXX_MODULES
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CLI11/src/modules)
