if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "CHESS_ENABLE_COVERAGE requires GCC and a matching gcov executable")
endif()

set(CHESS_COVERAGE_MINIMUM 95 CACHE STRING "Minimum total production line coverage percentage")
if(NOT CHESS_COVERAGE_MINIMUM MATCHES "^[0-9]+([.][0-9]+)?$"
        OR CHESS_COVERAGE_MINIMUM LESS_EQUAL 0 OR CHESS_COVERAGE_MINIMUM GREATER 100)
    message(FATAL_ERROR "CHESS_COVERAGE_MINIMUM must be greater than 0 and at most 100")
endif()

find_program(GCOVR_EXECUTABLE NAMES gcovr)
if(NOT GCOVR_EXECUTABLE)
    message(FATAL_ERROR "Install coverage tools with: python -m pip install -r requirements-coverage.txt")
endif()
string(REGEX MATCH "^[0-9]+" gcc_major "${CMAKE_CXX_COMPILER_VERSION}")
find_program(GCOV_EXECUTABLE NAMES "gcov-${gcc_major}" gcov)
if(NOT GCOV_EXECUTABLE)
    message(FATAL_ERROR "Install the gcov executable matching your GCC compiler")
endif()

add_library(chess_coverage INTERFACE)
# The GUI runs engine searches on worker threads. Atomic counters avoid corrupt
# coverage counts when the same engine code is used concurrently.
target_compile_options(chess_coverage INTERFACE --coverage -O0 -g -fprofile-update=atomic)
target_link_options(chess_coverage INTERFACE --coverage)

function(chess_add_coverage_target)
    set(report_dir "${CMAKE_BINARY_DIR}/coverage")
    set(report_title "Chess coverage - line coverage goal: ${CHESS_COVERAGE_MINIMUM}%")
    add_custom_target(coverage
        COMMAND "${CMAKE_COMMAND}" "-DCOVERAGE_BINARY_DIR=${CMAKE_BINARY_DIR}"
            -P "${CMAKE_SOURCE_DIR}/cmake/ResetCoverage.cmake"
        COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure --no-tests=error -C "$<CONFIG>"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${report_dir}"
        COMMAND "${GCOVR_EXECUTABLE}"
            --config "${CMAKE_SOURCE_DIR}/gcovr.cfg"
            --root "${CMAKE_SOURCE_DIR}"
            --gcov-executable "${GCOV_EXECUTABLE}"
            --html-details "${report_dir}/index.html" --html-title "${report_title}"
            --html-single-page js-enabled --html-self-contained
            --markdown "${report_dir}/summary.md" --markdown-title "${report_title}"
            --txt "${report_dir}/coverage.txt"
            --lcov "${report_dir}/coverage.info" --lcov-format-1.x
            --xml "${report_dir}/coverage.xml"
            --json-summary "${report_dir}/summary.json"
            --medium-threshold-line "${CHESS_COVERAGE_MINIMUM}"
            --high-threshold-line "${CHESS_COVERAGE_MINIMUM}"
            --fail-under-line "${CHESS_COVERAGE_MINIMUM}"
            --print-summary "${CMAKE_BINARY_DIR}"
        DEPENDS Chess position_test sliding_piece_test ai_test application_test
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Run all tests and report production line coverage (goal ${CHESS_COVERAGE_MINIMUM}%)"
        USES_TERMINAL
        VERBATIM
    )
endfunction()
