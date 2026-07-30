# Run the real Doxyfile (same config as the published docs: impl/ headers,
# book chapters, doxygen-awesome styling). All paths in the Doxyfile are
# relative to the repo root, and OUTPUT_DIRECTORY is empty, so the output
# lands in html/ at the repo root (gitignored).
add_custom_target(
  docs
  COMMAND ${CMAKE_COMMAND} -E env CLI11_VERSION=${CLI11_VERSION} $<TARGET_FILE:Doxygen::doxygen>
          "${PROJECT_SOURCE_DIR}/docs/Doxyfile"
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  COMMENT "Generating API documentation with Doxygen (output: ${PROJECT_SOURCE_DIR}/html)"
  VERBATIM)
