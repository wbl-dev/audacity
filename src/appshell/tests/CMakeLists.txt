#
# Audacity: A Digital Audio Editor
#

set(MODULE_TEST appshell_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/factoryreset_actioncontroller_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workbench_menu_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workbench_startup_tests.cpp

    ${CMAKE_CURRENT_LIST_DIR}/mocks/appshellconfigurationmock.h
)

set(MODULE_TEST_LINK
    appshell
    project

    muse::ui
)

set(MODULE_TEST_INCLUDE
    ${MUSE_FRAMEWORK_PATH}/framework/multiwindows
    ${MUSE_FRAMEWORK_PATH}/framework/actions
    ${MUSE_FRAMEWORK_PATH}/framework/interactive
    ${MUSE_FRAMEWORK_PATH}/framework/global
    ${MUSE_FRAMEWORK_PATH}/framework/rcommand
    ${CMAKE_SOURCE_DIR}/src/appshell
    ${CMAKE_SOURCE_DIR}/src/record
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)

add_test(
    NAME workbench_page_manifest
    COMMAND appshell_tests --gtest_filter=workbench_page_manifest.*
)
add_test(
    NAME workbench_startup
    COMMAND appshell_tests --gtest_filter=workbench_startup.*
)
add_test(
    NAME workbench_menu
    COMMAND appshell_tests --gtest_filter=workbench_menu.*
)
