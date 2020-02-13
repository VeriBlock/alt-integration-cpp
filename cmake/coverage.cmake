set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Og")

include(${CMAKE_CURRENT_LIST_DIR}/third_party/CodeCoverage.cmake)

append_coverage_compiler_flags()

set(COVERAGE_EXCLUDES
        '${CMAKE_SOURCE_DIR}/deps/*'
        '${CMAKE_SOURCE_DIR}/build/*'
        '${CMAKE_SOURCE_DIR}/cmake-build-*/*'
        '/usr/include/*'
        )

setup_target_for_coverage_gcovr_xml(
        NAME ctest_coverage
        EXECUTABLE ctest
)

setup_target_for_coverage_gcovr_html(
        NAME ctest_coverage_html
        EXECUTABLE ctest
)