cmake_minimum_required(VERSION 3.29.1)
project(
        Thoth
        VERSION 0.4.0
        DESCRIPTION "A simple functional webdev library"
        LANGUAGES CXX
)

#region Settings
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_DEBUG_POSTFIX "_d")

set(PROJECT_VERSION_SUFFIX "")
set(PROJECT_VERSION_FULL "${PROJECT_VERSION}${PROJECT_VERSION_SUFFIX}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
#endregion

#region options
option(THOTH_BUILD_TESTS      "Build Thoth tests"             ${PROJECT_IS_TOP_LEVEL})
option(THOTH_BUILD_EXAMPLES   "Build Thoth examples"          ${PROJECT_IS_TOP_LEVEL})
option(THOTH_INSTALL          "Generate the install target."  ${PROJECT_IS_TOP_LEVEL})
option(THOTH_BUILD_BENCHMARKS "Build Thoth benchmarks"        OFF)
option(THOTH_BUILD_DOCS       "Build Doxygen documentation"   OFF)
#endregion

#region top-level compiler options
add_library(ThothCompilerFlags INTERFACE)
if(PROJECT_IS_TOP_LEVEL)
    if(MSVC)
        target_compile_options(ThothCompilerFlags INTERFACE
                $<IF:$<CONFIG:Release>,/permissive- /O2 /GL /Oi /Ot /GT /Gw /Gy /Zc:inline /openmp:llvm,>
        )
        target_link_options(ThothCompilerFlags INTERFACE $<IF:$<CONFIG:Release>,/LTCG,>)
    endif()
endif()
#endregion

enable_testing()
include(cmake/GetCPM.cmake)
include(cmake/Macros.cmake)
register_lldb_script(Thoth)

add_library(Thoth
        src/Thoth/Http/Middleware/Decompress.cpp
        src/Thoth/Http/Middleware/Retry.cpp)
add_library(Thoth::Thoth ALIAS Thoth)

set_target_properties(Thoth PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
        POSITION_INDEPENDENT_CODE            ON
)

#region Thoth Files
file(GLOB_RECURSE THOTH_PUBLIC_HEADERS CONFIGURE_DEPENDS include/Thoth/*.hpp include/Thoth/*.tpp)

target_sources(Thoth PRIVATE
        src/Thoth/Utils/Env.cpp

        src/Thoth/NJson/Json.cpp
        src/Thoth/NJson/JsonObject.cpp
        src/Thoth/NJson/StringRef.cpp
        src/Thoth/NJson/Number.cpp

        src/Thoth/Http/Url/Url.cpp
        src/Thoth/Http/Request/QueryParams.cpp

        src/Thoth/Http/HttpMethods/GetHttpMethod.cpp
        src/Thoth/Http/HttpMethods/_base/HttpMethodBase.cpp

        src/Thoth/Http/NHeaders/Headers.cpp
        src/Thoth/Http/NHeaders/Headers/MimeType.cpp

        src/Thoth/Http/NHeaders/Request/RequestHeaders.cpp
        src/Thoth/Http/NHeaders/Response/ResponseHeaders.cpp

        src/Thoth/Http/Client.cpp
        src/Thoth/Http/Request/Request.cpp
        src/Thoth/String/Utils.cpp
        PUBLIC
        FILE_SET headers
        TYPE HEADERS
        BASE_DIRS include
        FILES ${THOTH_PUBLIC_HEADERS}
)
#endregion

#region linking
CPMFindPackage(
        NAME                    Hermes
        VERSION                 0.5.6
        GITHUB_REPOSITORY       NotRiemannCousin/Hermes
        GIT_TAG                 v0.5.6
        OPTIONS                 "HERMES_INSTALL ${THOTH_INSTALL}"
        FIND_PACKAGE_ARGUMENTS  "CONFIG"
)

CPMFindPackage(
        NAME                    ZLIB
        VERSION                 1.3.2
        GITHUB_REPOSITORY       madler/zlib
        GIT_TAG                 v1.3.2
        OPTIONS                 "ZLIB_BUILD_EXAMPLES OFF"
                                "ZLIB_BUILD_TESTING OFF"
                                "ZLIB_BUILD_SHARED OFF"
                                "ZLIB_INSTALL ${THOTH_INSTALL}"
)

if(TARGET zlibstatic AND NOT TARGET ZLIB::ZLIB)
    add_library(ZLIB::ZLIB ALIAS zlibstatic)
endif()

target_link_libraries(Thoth PUBLIC  Hermes::Hermes)
target_link_libraries(Thoth PRIVATE ZLIB::ZLIB)

if(TARGET ThothCompilerFlags)
    target_link_libraries(Thoth PRIVATE $<BUILD_INTERFACE:ThothCompilerFlags>)
endif()
#endregion

add_subdirectory_if(THOTH_BUILD_TESTS      tests)
add_subdirectory_if(THOTH_BUILD_EXAMPLES   examples)
add_subdirectory_if(THOTH_BUILD_BENCHMARKS benchmarks)
add_subdirectory_if(THOTH_BUILD_DOCS       docs)

#region install
if(THOTH_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(THOTH_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}@${PROJECT_VERSION_FULL}")

    install(TARGETS Thoth
            EXPORT ThothTargets
            FILE_SET headers
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(EXPORT ThothTargets
            FILE ThothTargets.cmake
            NAMESPACE Thoth::
            DESTINATION ${THOTH_INSTALL_CMAKEDIR}
    )

    write_basic_package_version_file(
            "${CMAKE_CURRENT_BINARY_DIR}/ThothConfigVersion.cmake"
            VERSION ${PROJECT_VERSION}
            COMPATIBILITY AnyNewerVersion
    )

    configure_package_config_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ThothConfig.cmake.in"
            "${CMAKE_CURRENT_BINARY_DIR}/ThothConfig.cmake"
            INSTALL_DESTINATION ${THOTH_INSTALL_CMAKEDIR}
    )

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/ThothConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/ThothConfigVersion.cmake"
            DESTINATION ${THOTH_INSTALL_CMAKEDIR}
    )
endif()
#endregion


generate_lldbinit()