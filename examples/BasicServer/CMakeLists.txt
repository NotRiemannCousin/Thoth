add_executable(BasicServerExample main.cpp)

find_package(Threads REQUIRED)

target_link_libraries(
        BasicServerExample
        Thoth::Thoth
        Threads::Threads
)
