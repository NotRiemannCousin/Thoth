add_executable(PostMortemExample
        src/main.cpp
        src/Accounts.cpp
        src/Board.cpp
        src/Html.cpp
)

target_include_directories(PostMortemExample PRIVATE include)

find_package(Threads REQUIRED)

target_link_libraries(PostMortemExample
        PRIVATE
        Thoth::Thoth
        Threads::Threads
)

# The components, the stylesheet and the script are read at runtime, they have to live next to the executable.
add_custom_command(TARGET PostMortemExample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/public"
        "$<TARGET_FILE_DIR:PostMortemExample>/public"
        COMMENT "Copying web files..."
)
