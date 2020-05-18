if(INSTALL_FMT)
    install(
            DIRECTORY ${CMAKE_SOURCE_DIR}/third_party/fmt
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()