cmake_minimum_required(VERSION 3.29.1)
project(Thoth)

set(PROJECT_VERSION_NUMBER 0.1.0)
set(PROJECT_VERSION_SUFFIX "alpha.1")

set(PROJECT_VERSION "${PROJECT_VERSION_NUMBER}-${PROJECT_VERSION_SUFFIX}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Hermes REQUIRED)

#--------------------------------------------------------------------------------------------------------------
#           Thoth
#--------------------------------------------------------------------------------------------------------------

add_library(Thoth SHARED
        src/Thoth/Http/Request/HttpRequest.cpp
        src/Thoth/Http/QueryParams.cpp
        src/Thoth/Http/HttpUrl.cpp
        include/Thoth/Http/Request/HttpUrl.hpp
        include/Thoth/Http/Request/HttpUrl.tpp
        include/Thoth/Http/HttpMethods/_base/HttpMethodConcept.hpp
        src/Thoth/Http/HttpHeaders.cpp
        include/Thoth/Http/HttpHeaders.hpp
        include/Thoth/Http/Response/HttpStatusCodeEnum.hpp
        include/Thoth/Http/Request/HttpRequest.tpp
        include/Thoth/Http/HttpClient.hpp
        include/Thoth/Http/HttpClient.tpp
        src/Thoth/Http/Request/HttpClient.cpp
        include/Thoth/Http/Response/HttpResponse.hpp
)

target_include_directories(
        Thoth
        PUBLIC

        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(Thoth PUBLIC Hermes::Hermes)

set_target_properties(Thoth PROPERTIES
        WINDOWS_EXPORT_ALL_SYMBOLS ON
)


#--------------------------------------------------------------------------------------------------------------
#           Examples
#--------------------------------------------------------------------------------------------------------------


add_executable(
        ThothExample

        examples/main.cpp
)

target_link_libraries(
        ThothExample
        PUBLIC

        Thoth
)


#--------------------------------------------------------------------------------------------------------------
#           Tests
#--------------------------------------------------------------------------------------------------------------


add_executable(
        ThothTests


        tests/main.cpp
        tests/src/HttpTests.cpp
)

target_include_directories(ThothTests PUBLIC tests/include)

target_link_libraries(
        ThothTests
        PUBLIC

        Thoth
)
