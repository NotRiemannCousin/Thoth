find_package(Doxygen REQUIRED)
add_custom_target(ThothDocs
        COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
        COMMENT "Generating API documentation..."
        VERBATIM
)