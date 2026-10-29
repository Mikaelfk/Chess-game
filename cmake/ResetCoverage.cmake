# Each report must describe this test run, not accumulate earlier/manual runs.
if(NOT DEFINED COVERAGE_BINARY_DIR OR NOT IS_DIRECTORY "${COVERAGE_BINARY_DIR}")
    message(FATAL_ERROR "COVERAGE_BINARY_DIR must name an existing build directory")
endif()
file(GLOB_RECURSE counters "${COVERAGE_BINARY_DIR}/*.gcda")
if(counters)
    file(REMOVE ${counters})
endif()
file(REMOVE_RECURSE "${COVERAGE_BINARY_DIR}/coverage")
