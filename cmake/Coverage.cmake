if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "CHESS_ENABLE_COVERAGE requires GCC and a matching gcov executable")
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
    set(report_title "Chess test coverage")
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
            --print-summary "${CMAKE_BINARY_DIR}"
        DEPENDS Chess position_test sliding_piece_test ai_test application_test mobile_game_test
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Run all tests and report production line coverage"
        USES_TERMINAL
        VERBATIM
    )
endfunction()
