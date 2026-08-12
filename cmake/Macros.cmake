function(add_subdirectory_if OPTION PATH)
    if(${OPTION})
        add_subdirectory(${PATH})
    endif()
endfunction()


macro(export_vendor_if_applicable VENDOR)
    string(TOUPPER "${PROJECT_NAME}" _evia_project_upper)
    string(TOUPPER "${VENDOR}"       _evia_vendor_upper)

    set(${_evia_project_upper}_VENDORS_${_evia_vendor_upper} OFF)

    if(${VENDOR}_ADDED)
        get_target_property(_evia_header_sets ${VENDOR} INTERFACE_HEADER_SETS)
        set(_evia_file_set_args "")

        if(_evia_header_sets)
            foreach(_evia_fs IN LISTS _evia_header_sets)
                list(APPEND _evia_file_set_args FILE_SET ${_evia_fs} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
            endforeach()
        endif()

        get_target_property(_evia_cxx_modules ${VENDOR} INTERFACE_CXX_MODULE_SETS)
        if(_evia_cxx_modules)
            foreach(_evia_fs IN LISTS _evia_cxx_modules)
                list(APPEND _evia_file_set_args FILE_SET ${_evia_fs} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
            endforeach()
        endif()

        install(TARGETS ${VENDOR}
                EXPORT ${PROJECT_NAME}Targets
                ${_evia_file_set_args}
        )

        if(NOT _evia_file_set_args AND EXISTS "${${VENDOR}_SOURCE_DIR}/include")
            install(DIRECTORY "${${VENDOR}_SOURCE_DIR}/include/"
                    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
        endif()

        set(${_evia_project_upper}_VENDORS_${_evia_vendor_upper} ON)
    endif()

    unset(_evia_header_sets)
    unset(_evia_cxx_modules)
    unset(_evia_file_set_args)
    unset(_evia_fs)
    unset(_evia_project_upper)
    unset(_evia_vendor_upper)
endmacro()