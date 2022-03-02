set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -O0")

include(${CMAKE_CURRENT_LIST_DIR}/third_party/CodeCoverage.cmake)

append_coverage_compiler_flags()

set(COVERAGE_EXCLUDES
        '${PROJECT_BINARY_DIR}/_deps/*'
        '${PROJECT_BINARY_DIR}/gens/*'
        '${CMAKE_SOURCE_DIR}/build/*'
        '${CMAKE_SOURCE_DIR}/cmake-build-*/*'
        '${CMAKE_SOURCE_DIR}/include/veriblock/pop/third_party/*'
        '/usr/include/*'
        )

setup_target_for_coverage_gcovr_xml(
        NAME ctest_coverage
        EXECUTABLE ctest -j ${PROCESSOR_COUNT}
        DEPENDENCIES all_test
        BASE_DIRECTORY ${PROJECT_BINARY_DIR}
        EXCLUDE ${COVERAGE_EXCLUDES}
)

setup_target_for_coverage_lcov(
        NAME ctest_coverage_lcov
        EXECUTABLE ctest -j ${PROCESSOR_COUNT}
        DEPENDENCIES all_tests
        BASE_DIRECTORY ${PROJECT_BINARY_DIR}
        EXCLUDE ${COVERAGE_EXCLUDES}
)

setup_target_for_coverage_gcovr_html(
        NAME ctest_coverage_html
        EXECUTABLE ctest -j ${PROCESSOR_COUNT}
        DEPENDENCIES all_tests
        BASE_DIRECTORY ${PROJECT_BINARY_DIR}
        EXCLUDE ${COVERAGE_EXCLUDES}
)
