#
# Audacity: A Digital Audio Editor
#

set(MODULE_TEST preferences_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/commonaudioapiconfigurationmodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workbench_general_preferences_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../qml/Audacity/Preferences/commonaudioapiconfigurationmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../qml/Audacity/Preferences/commonaudioapiconfigurationmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/../qml/Audacity/Preferences/generalpreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../qml/Audacity/Preferences/generalpreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/../qml/Audacity/Preferences/updatepreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../qml/Audacity/Preferences/updatepreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/../qml/Audacity/Preferences/usageinfopreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../qml/Audacity/Preferences/usageinfopreferencesmodel.h
    )

set(MODULE_TEST_LINK
    audio
    muse_update
    usageinfo
    Qt6::Qml
    Qt6::Quick
    )

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)
set_target_properties(${MODULE_TEST} PROPERTIES AUTOMOC ON)

add_test(
    NAME workbench_general_preferences
    COMMAND preferences_tests --gtest_filter=workbench_general_preferences.*
)
