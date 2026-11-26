#
# Audacity: A Digital Audio Editor
#

set(MODULE_TEST uicomponents_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/timecodemodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/popupviewclosecontroller_tests.cpp
)

if (OS_IS_MAC)
    list(APPEND MODULE_TEST_SRC ${CMAKE_CURRENT_LIST_DIR}/macospopupviewclosecontroller_tests.mm)
endif()

set(MODULE_TEST_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/..)

set(MODULE_TEST_LINK
    uicomponents
    uicomponents_qml

    muse::uicomponents
    muse::ui

    Qt::Quick
    )

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})
include(SetupGTest)
