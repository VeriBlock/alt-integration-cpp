include_directories(${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(blockchain)
add_subdirectory(storage)
add_subdirectory(rewards)
add_subdirectory(third_party)
add_subdirectory(entities)
add_subdirectory(crypto)

add_library(vblake OBJECT crypto/vblake.cpp)

add_library(strutil OBJECT
        base58.cpp
        base59.cpp
        strutil.cpp
        )

add_library(serde OBJECT
        serde.cpp
        read_stream.cpp
        write_stream.cpp
        )

add_library(mempool OBJECT
        mempool.cpp
        mempool_relations.cpp
        )

add_library(version OBJECT
        version.cpp
        )
target_compile_definitions(version PRIVATE VBK_VERSION=${VERSION})
target_add_flag_visibility(version PRIVATE
        -Wno-unused-variable
        )

add_library(pop_stateless_validator OBJECT
        pop_stateless_validator.cpp
        )
if(VBK_NO_THREADS)
    add_compile_definitions(VBK_NO_THREADS)
endif()

set(SOURCES
        validation_state.cpp
        bootstraps.cpp
        time.cpp
        keystone_util.cpp
        hashutil.cpp
        stateless_validation.cpp
        arith_uint256.cpp
        crypto/secp256k1.cpp
        mock_miner.cpp
        config.cpp
        command_group_cache.cpp
        alt-util.cpp
        pop_context.cpp
        $<TARGET_OBJECTS:mempool>
        $<TARGET_OBJECTS:pop_stateless_validator>
        $<TARGET_OBJECTS:strutil>
        $<TARGET_OBJECTS:serde>
        $<TARGET_OBJECTS:blockchain>
        $<TARGET_OBJECTS:pop_utils>
        $<TARGET_OBJECTS:storage>
        $<TARGET_OBJECTS:poprewards>
        $<TARGET_OBJECTS:entities>
        $<TARGET_OBJECTS:sha256>
        $<TARGET_OBJECTS:vblake>
        $<TARGET_OBJECTS:progpow>
        $<TARGET_OBJECTS:ethash>
        )

if(WITH_C_WRAPPER)
    add_subdirectory(c)
    list(APPEND SOURCES $<TARGET_OBJECTS:veriblock-pop-c>)
endif()

if(WITH_SECP256K1)
    list(APPEND SOURCES $<TARGET_OBJECTS:secp256k1>)
endif()

if(VERIBLOCK_POP_LOGGER_ENABLED)
    list(APPEND SOURCES logger.cpp)
endif()

if(SHARED)
    set(BUILD SHARED)
else()
    set(BUILD STATIC)
endif()
message(STATUS "BUILD=${BUILD}")

add_library(${LIB_NAME} ${BUILD} ${SOURCES})

if(WITH_TRACER)
    target_link_libraries(${LIB_NAME} tracy)
endif()

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

# make install is not defined with FUZZING=ON
if(NOT FUZZING AND NOT WITH_TRACER)
    install(TARGETS ${LIB_NAME}
            EXPORT ${LIB_NAME}Config
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
            EXPORT ${LIB_NAME}Config
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}
    )
    export(
            TARGETS ${LIB_NAME}
            FILE ${LIB_NAME}Config.cmake
    )
endif()
target_include_directories(${LIB_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

if(WITH_ROCKSDB)
        set(VBK_DEPENDENCIES_LIBS -lrocksdb)
endif()

if(WITH_LEVELDB)
        set(VBK_DEPENDENCIES_LIBS -lleveldb)
endif()

if(ASAN)
        set(SANITIZERS_LIBS "-fsanitize=address")
        set(SANITIZERS_CFLAGS "-fsanitize=address -DWITH_ASAN")
endif()

set(configured_pc ${CMAKE_BINARY_DIR}/${LIB_NAME}.pc)
configure_file("${CMAKE_SOURCE_DIR}/cmake/lib.pc.in" "${configured_pc}" @ONLY)
install(FILES "${configured_pc}"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")


enable_asan_on_target(${LIB_NAME})
enable_tsan_on_target(${LIB_NAME})
enable_ubsan_on_target(${LIB_NAME})
