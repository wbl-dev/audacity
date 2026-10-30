#
# Audacity: A Digital Audio Editor
#

set(MODULE_TEST app_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/workbench_module_policy_tests.cpp
)

set(MODULE_TEST_LINK
    muse::network
    shared
    Qt6::Core
)

set(MODULE_TEST_INCLUDE
    ${CMAKE_SOURCE_DIR}/src/app
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src/app
    ${MUSE_FRAMEWORK_PATH}/framework
)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)

add_test(
    NAME workbench_module_policy
    COMMAND app_tests --gtest_filter=workbench_module_policy.*
)
add_test(
    NAME workbench_network_stub
    COMMAND app_tests --gtest_filter=workbench_network_stub.*
)
