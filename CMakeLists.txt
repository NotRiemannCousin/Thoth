cmake_minimum_required(VERSION 3.29.1)
project(Thoth)

set(PROJECT_VERSION_NUMBER 0.2.0)
#set(PROJECT_VERSION_SUFFIX "alpha.1")

#set(PROJECT_VERSION "${PROJECT_VERSION_NUMBER}-${PROJECT_VERSION_SUFFIX}")
set(PROJECT_VERSION "${PROJECT_VERSION_NUMBER}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Hermes REQUIRED)

#--------------------------------------------------------------------------------------------------------------
#           Thoth
#--------------------------------------------------------------------------------------------------------------

add_library(ThothUtils INTERFACE)
target_include_directories(ThothUtils
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)


add_library(ThothJson SHARED
        src/Thoth/Json/Json.cpp
)

target_link_libraries(ThothJson PUBLIC ThothUtils)

set_target_properties(ThothJson PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)




add_library(ThothHttp SHARED
        src/Thoth/Http/HttpHeaders.cpp
        src/Thoth/Http/HttpUrl.cpp
        src/Thoth/Http/QueryParams.cpp

        src/Thoth/Http/HttpMethods/GetHttpMethod.cpp
        src/Thoth/Http/HttpMethods/_base/HttpMethodBase.cpp

        src/Thoth/Http/Request/HttpClient.cpp
        src/Thoth/Http/Request/HttpRequest.cpp
)

target_link_libraries(ThothHttp PUBLIC Hermes::Hermes ThothJson)

set_target_properties(ThothHttp PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)


#--------------------------------------------------------------------------------------------------------------
#           Examples
#--------------------------------------------------------------------------------------------------------------


add_executable(ThothExampleRequest1 examples/Request1.cpp)
add_executable(ThothExampleRequest2 examples/Request2.cpp)

add_executable(ThothExampleJson1 examples/Json1.cpp)


target_link_libraries(ThothExampleRequest1 PUBLIC ThothHttp)
target_link_libraries(ThothExampleRequest2 PUBLIC ThothHttp)

target_link_libraries(ThothExampleJson1 PUBLIC ThothJson)


#--------------------------------------------------------------------------------------------------------------
#           Tests
#--------------------------------------------------------------------------------------------------------------


add_executable(
        ThothTests


        tests/main.cpp
        tests/src/HttpTests.cpp
)

target_include_directories(ThothTests PUBLIC tests/include)

target_link_libraries(ThothTests PUBLIC ThothHttp)
