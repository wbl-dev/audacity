#
# Audacity: A Digital Audio Editor
#

set(MODULE_TEST projectscene_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/projectsceneconfiguration_migration_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp
    ${CMAKE_CURRENT_LIST_DIR}/projectviewstate_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sample_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/snaptimeformatter_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tracklabelslayoutmanager_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/trackitemsselection_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/trackitemsmovecontroller_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/findguideline_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/iconfontresources_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playcursorcontroller_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playpositionactioncontroller_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbacktoolbarmodel_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/timelinecontext_labelzoom_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/projectoverview_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/trackitem_color_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/marqueeselection_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/selectionbelowtracks_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/doubleclickselection_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dropcontroller_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workbenchgesturemath_tests.cpp

    ${CMAKE_CURRENT_LIST_DIR}/snaptestaccess.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/globalcontextmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/agu4projectmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/projectsceneconfigurationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/audiooutputmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/projectviewstatemock.h
)

    list(APPEND MODULE_TEST_SRC
        ${CMAKE_CURRENT_LIST_DIR}/workbench_online_surfaces_tests.cpp
    )

set(MODULE_TEST_LINK
    projectscene
    trackedit
    playback
    record
    context
    au3wrap

    muse::ui
    muse::uicomponents
    muse::workspace
)

set(MODULE_TEST_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/..)

set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)

    add_test(
        NAME workbench_online_surfaces_projectscene
        COMMAND projectscene_tests --gtest_filter=workbench_online_surfaces_projectscene.*
    )
