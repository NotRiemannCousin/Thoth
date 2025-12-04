add_executable(ThothExampleRequest1 Request1.cpp)
add_executable(ThothExampleRequest2 Request2.cpp)

add_executable(ThothExampleJson1 Json1.cpp)

add_executable(ThothExampleEnv1 Env1/Env.cpp)
add_executable(ThothExampleEnv2 Env2/Env.cpp)

add_executable(VsNlohmann VsNlohmann/main.cpp)
add_executable(YoutubeJson1 YoutubeJson1.cpp)
add_executable(YoutubeJson2 YoutubeJson2.cpp)


#target_compile_definitions(ThothExampleRequest1 PRIVATE -DDENSE_DEBUG_JSON)


target_link_libraries(ThothExampleRequest1 PUBLIC Thoth)
target_link_libraries(ThothExampleRequest2 PUBLIC Thoth)

target_link_libraries(ThothExampleJson1 PUBLIC Thoth)

target_link_libraries(ThothExampleEnv1 PUBLIC Thoth)
target_link_libraries(ThothExampleEnv2 PUBLIC Thoth)

target_link_libraries(VsNlohmann   PUBLIC Thoth)
target_link_libraries(YoutubeJson1 PUBLIC Thoth)
target_link_libraries(YoutubeJson2 PUBLIC Thoth)
