set(CLI11_headerLoc "${PROJECT_SOURCE_DIR}/include/CLI")

set(CLI11_headers
    ${CLI11_headerLoc}/App.hpp
    ${CLI11_headerLoc}/CLI.hpp
    ${CLI11_headerLoc}/Config.hpp
    ${CLI11_headerLoc}/ConfigFwd.hpp
    ${CLI11_headerLoc}/Error.hpp
    ${CLI11_headerLoc}/Formatter.hpp
    ${CLI11_headerLoc}/FormatterFwd.hpp
    ${CLI11_headerLoc}/Macros.hpp
    ${CLI11_headerLoc}/Option.hpp
    ${CLI11_headerLoc}/Split.hpp
    ${CLI11_headerLoc}/StringTools.hpp
    ${CLI11_headerLoc}/Timer.hpp
    ${CLI11_headerLoc}/TypeTools.hpp
    ${CLI11_headerLoc}/Validators.hpp
    ${CLI11_headerLoc}/Version.hpp)

set(CLI11_implLoc "${PROJECT_SOURCE_DIR}/include/CLI/impl")

set(CLI11_impl_headers
    ${CLI11_implLoc}/App_inl.hpp
    ${CLI11_implLoc}/Config_inl.hpp
    ${CLI11_implLoc}/Formatter_inl.hpp
    ${CLI11_implLoc}/Option_inl.hpp
    ${CLI11_implLoc}/Split_inl.hpp
    ${CLI11_implLoc}/StringTools_inl.hpp
    ${CLI11_implLoc}/Validators_inl.hpp)

if(CLI11_PRECOMPILED)
  # Create static lib
  file(GLOB CLI11_precompile_sources "${PROJECT_SOURCE_DIR}/src/*.cpp")
  add_library(CLI11 STATIC ${CLI11_headers} ${CLI11_impl_headers} ${CLI11_precompile_sources})
  target_compile_definitions(CLI11 PUBLIC -DCLI11_COMPILE)

  set(PUBLIC_OR_INTERFACE PUBLIC)
else()
  add_library(CLI11 INTERFACE)
  if(CMAKE_VERSION VERSION_GREATER 3.19)
    # This is only useful for visual studio and other IDE builds
    target_sources(CLI11 PRIVATE ${CLI11_headers} ${CLI11_impl_headers})
  endif()

  set(PUBLIC_OR_INTERFACE INTERFACE)
endif()

# Allow IDE's to group targets into folders
add_library(CLI11::CLI11 ALIAS CLI11) # for add_subdirectory calls

# Duplicated because CMake adds the current source dir if you don't.
target_include_directories(
  CLI11 ${PUBLIC_OR_INTERFACE} $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

if(CMAKE_VERSION VERSION_LESS 3.8)
  # This might not be a complete list
  target_compile_features(
    CLI11
    INTERFACE cxx_lambdas
              cxx_nullptr
              cxx_override
              cxx_range_for
              cxx_right_angle_brackets
              cxx_strong_enums
              cxx_constexpr
              cxx_auto_type)
else()
  target_compile_features(CLI11 INTERFACE cxx_std_11)
endif()

if(CLI11_SINGLE_FILE)
  # Single file test
  if(CMAKE_VERSION VERSION_LESS 3.12)
    find_package(PythonInterp REQUIRED)
    add_executable(Python::Interpreter IMPORTED)
    set_target_properties(Python::Interpreter PROPERTIES IMPORTED_LOCATION "${PYTHON_EXECUTABLE}"
                                                         VERSION "${PYTHON_VERSION_STRING}")
  else()
    find_package(
      Python
      COMPONENTS Interpreter
      REQUIRED)
  endif()

  file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/include")
  add_custom_command(
    OUTPUT "${PROJECT_BINARY_DIR}/include/CLI11.hpp"
    COMMAND
      Python::Interpreter "${PROJECT_SOURCE_DIR}/scripts/MakeSingleHeader.py" ${CLI11_headers}
      ${CLI11_impl_headers} --main "${PROJECT_SOURCE_DIR}/CLI11.hpp.in" --output
      "${PROJECT_BINARY_DIR}/include/CLI11.hpp" --version "${CLI11_VERSION}"
    DEPENDS "${PROJECT_SOURCE_DIR}/include/CLI/CLI.hpp" ${CLI11_headers} ${CLI11_impl_headers})
  add_custom_target(CLI11-generate-single-file ALL
                    DEPENDS "${PROJECT_BINARY_DIR}/include/CLI11.hpp")
  set_property(TARGET CLI11-generate-single-file PROPERTY FOLDER "Scripts")
  if(CLI11_INSTALL)
    install(FILES "${PROJECT_BINARY_DIR}/include/CLI11.hpp"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  endif()
  add_library(CLI11_SINGLE INTERFACE)
  target_link_libraries(CLI11_SINGLE INTERFACE CLI11)
  add_dependencies(CLI11_SINGLE CLI11-generate-single-file)
  target_compile_definitions(CLI11_SINGLE INTERFACE -DCLI11_SINGLE_FILE)
  target_include_directories(
    CLI11_SINGLE INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
endif()

if(CLI11_INSTALL)

  # Make an export target
  install(TARGETS CLI11 EXPORT CLI11Targets)
  if(NOT CLI11_SINGLE_FILE)
    install(FILES ${CLI11_headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/CLI")
    if(NOT CLI11_COMPILE)
      install(FILES ${CLI11_impl_headers} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/CLI/impl")
    endif()
  endif()
endif()
