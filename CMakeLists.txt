cmake_minimum_required(VERSION 3.29.1)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_DEBUG_POSTFIX "_d")
project(Thoth)


add_compile_options(/favor:AMD64)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(
            /permissive-    # without MSVC madness
            /O2             # Hard Optimization
            /GL             # Link-time code generation
            /arch:AVX2      # SIMD
            /Oi             # Intrinsics
            /Ot             # Speed
            /GT             # Thread-safe static init
            /Gw /Gy         # Delete unused
            /Zc:inline      # inlining
            /openmp:llvm    # OpenMP
    )
else()
    add_link_options(
#            /LTCG           # Link-time optimization
#            /OPT:REF        # Delete unused
#            /OPT:ICF        # join equal functions
    )
endif()

set(PROJECT_VERSION_NUMBER 0.2.0)
set(PROJECT_VERSION_SUFFIX "alpha.1")

set(PROJECT_VERSION "${PROJECT_VERSION_NUMBER}-${PROJECT_VERSION_SUFFIX}")
set(PROJECT_VERSION "${PROJECT_VERSION_NUMBER}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Hermes REQUIRED)

#--------------------------------------------------------------------------------------------------------------
#           Thoth
#--------------------------------------------------------------------------------------------------------------

add_library(Thoth SHARED
        src/Thoth/Utils/Env.cpp

        src/Thoth/NJson/Json.cpp
        src/Thoth/NJson/JsonObject.cpp
        src/Thoth/NJson/StringRef.cpp

        src/Thoth/Http/Headers.cpp
        src/Thoth/Http/Request/Url.cpp
        src/Thoth/Http/Request/QueryParams.cpp

        src/Thoth/Http/HttpMethods/GetHttpMethod.cpp
        src/Thoth/Http/HttpMethods/_base/HttpMethodBase.cpp

        src/Thoth/Http/Client.cpp
        src/Thoth/Http/Request/Request.cpp
)

target_include_directories(Thoth
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

#target_compile_definitions(ThothJson PRIVATE -DDENSE_DEBUG_JSON)

target_link_libraries(Thoth PUBLIC Hermes::Hermes)

set_target_properties(Thoth PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)


add_subdirectory(examples)
add_subdirectory(tests)


install(TARGETS Thoth
        EXPORT ThothTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        CONFIGURATIONS Debug Release
)


install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)

install(EXPORT ThothTargets
        FILE ThothTargets.cmake
        NAMESPACE Thoth::
        DESTINATION lib/cmake/Thoth
        CONFIGURATIONS Debug Release
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/ThothConfigVersion.cmake"
        VERSION ${PROJECT_VERSION_NUMBER}
        COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ThothConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ThothConfig.cmake"
        INSTALL_DESTINATION lib/cmake/Thoth
)


install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ThothConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ThothConfigVersion.cmake"
        DESTINATION lib/cmake/Thoth
)