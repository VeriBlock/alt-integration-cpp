include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(blockchain)
add_subdirectory(third_party)
add_subdirectory(entities)
add_subdirectory(storage)

add_library(vblake OBJECT vblake.cpp)

add_library(strutil OBJECT
        base58.cpp
        base59.cpp
        strutil.cpp
        )

if(SHARED)
    set(BUILD SHARED)
else()
    set(BUILD STATIC)
endif()
message(STATUS "BUILD=${BUILD}")

add_library(${LIB_NAME} ${BUILD}
        ${CMAKE_SOURCE_DIR}/include/veriblock/alt-util.hpp
        popmanager.cpp
        checks.cpp
        serde.cpp
        keystone_util.cpp
        read_stream.cpp
        write_stream.cpp
        hashutil.cpp
        stateless_validation.cpp
        arith_uint256.cpp
        signutil.cpp
        mock_miner.cpp
        $<TARGET_OBJECTS:strutil>
        $<TARGET_OBJECTS:blockchain>
        $<TARGET_OBJECTS:pop_utils>
        $<TARGET_OBJECTS:entities>
        $<TARGET_OBJECTS:secp256k1>
        $<TARGET_OBJECTS:sha256>
        $<TARGET_OBJECTS:bigdecimal>
        $<TARGET_OBJECTS:vblake>
        )
if(WITH_ROCKS)
    target_link_libraries(${LIB_NAME} rocksdb)
endif()

# install step is only for unix, for now...

set_target_properties(${LIB_NAME} PROPERTIES
        VERSION ${VERSION}
        SOVERSION ${MAJOR_VERSION}
        INTERFACE_${LIB_NAME}_MAJOR_VERSION ${MAJOR_VERSION}
        )
set_property(
        TARGET ${LIB_NAME}
        APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING MAJOR_VERSION
)
install(TARGETS ${LIB_NAME}
        EXPORT ${LIB_NAME}Config
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
        EXPORT ${LIB_NAME}Config
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}
        NAMESPACE veriblock::
)
export(
        TARGETS ${LIB_NAME}
        FILE ${LIB_NAME}Config.cmake
)
target_include_directories(${LIB_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

set(configured_pc ${CMAKE_BINARY_DIR}/${LIB_NAME}.pc)
configure_file("${CMAKE_SOURCE_DIR}/cmake/lib.pc.in" "${configured_pc}" @ONLY)
install(FILES "${configured_pc}"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
