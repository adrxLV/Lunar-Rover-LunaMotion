# ---- Variables ----

# We use variables separate from what CTest uses, because those have
# customization issues
set(
    COVERAGE_TRACE_COMMAND
    gcovr --exclude-unreachable-branches --print-summary
    --root "${PROJECT_SOURCE_DIR}"
    --gcov-object-directory "${PROJECT_BINARY_DIR}"
    CACHE STRING
    "; separated command to generate a trace for the 'coverage' target"
)

set(
    COVERAGE_HTML_COMMAND
    gcovr --html-details --exclude-unreachable-branches
    --root "${PROJECT_SOURCE_DIR}"
    --gcov-object-directory "${PROJECT_BINARY_DIR}"
    --output "${PROJECT_BINARY_DIR}/coverage_html/"
    CACHE STRING
    "; separated command to generate an HTML report for the 'coverage' target"
)

# ---- Coverage target ----

add_custom_target(
    coverage
    COMMAND ${COVERAGE_TRACE_COMMAND}
    COMMAND ${COVERAGE_HTML_COMMAND}
    COMMENT "Generating coverage report"
    VERBATIM
)
