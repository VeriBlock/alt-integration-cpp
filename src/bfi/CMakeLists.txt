add_subdirectory(veriblock)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(SOURCES 
        dummy.cpp
        )

add_library(${BFI_LIB_NAME} ${BUILD} ${SOURCES})
enable_clang_tidy(${BFI_LIB_NAME})

target_link_libraries(${BFI_LIB_NAME} ${POP_LIB_NAME})

set_target_properties(${BFI_LIB_NAME} PROPERTIES
        VERSION ${VERSION}
        SOVERSION ${MAJOR_VERSION}
        INTERFACE_${BFI_LIB_NAME}_MAJOR_VERSION ${MAJOR_VERSION}
        )
set_property(
        TARGET ${BFI_LIB_NAME}
        APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING MAJOR_VERSION
)

# make install is not defined with FUZZING=ON
if(NOT FUZZING)
    install(TARGETS ${BFI_LIB_NAME}
            EXPORT ${BFI_LIB_NAME}Config
            COMPONENT devel
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FRAMEWORK DESTINATION ${CMAKE_INSTALL_PREFIX}
            )
    install(
            DIRECTORY ${CMAKE_SOURCE_DIR}/include/veriblock
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
            EXPORT ${BFI_LIB_NAME}Config
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${BFI_LIB_NAME}
    )
    export(
            TARGETS ${BFI_LIB_NAME}
            FILE ${BFI_LIB_NAME}Config.cmake
    )
endif()

target_include_directories(${BFI_LIB_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )


enable_asan_on_target(${BFI_LIB_NAME})
enable_tsan_on_target(${BFI_LIB_NAME})
enable_ubsan_on_target(${BFI_LIB_NAME})
enable_werror(${BFI_LIB_NAME})
enable_stacktrace_on_target(${BFI_LIB_NAME})