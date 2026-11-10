set(MODULE_TEST labels_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/labelscriptengine_tests.cpp
)

set(MODULE_TEST_LINK
    iex_labels
)

include(SetupGTest)

target_compile_definitions(labels_tests PRIVATE LABELSCRIPTWORKER_PATH="$<TARGET_FILE:labelscriptworker>")
add_dependencies(labels_tests labelscriptworker)
