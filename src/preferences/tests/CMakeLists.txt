#
# Audacity: A Digital Audio Editor
#

set(MODULE_TEST preferences_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/commonaudioapiconfigurationmodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../qml/AGu/Preferences/commonaudioapiconfigurationmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../qml/AGu/Preferences/commonaudioapiconfigurationmodel.h
    )

    list(APPEND MODULE_TEST_SRC
        ${CMAKE_CURRENT_LIST_DIR}/workbench_general_preferences_tests.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../qml/AGu/Preferences/generalpreferencesmodel.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../qml/AGu/Preferences/generalpreferencesmodel.h
    )

set(MODULE_TEST_LINK
    audio
    Qt6::Qml
    Qt6::Quick
    )

    list(APPEND MODULE_TEST_LINK
    )

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)
set_target_properties(${MODULE_TEST} PROPERTIES AUTOMOC ON)

    add_test(
        NAME workbench_general_preferences
        COMMAND preferences_tests --gtest_filter=workbench_general_preferences.*
    )
