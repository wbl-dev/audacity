#
# Audacity: A Digital Audio Editor
#

set(MODULE_TEST au_project_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp

    ${CMAKE_CURRENT_LIST_DIR}/mocks/aguprojectmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/projectconfigurationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/trackeditprojectcreatormock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/dummyeffectinstancefactory.h
    ${CMAKE_CURRENT_LIST_DIR}/aguproject_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/projectrecovery_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/realtimeeffectspersistence_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cliptempopersistence_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/au3metadata_tests.cpp
)

    list(APPEND MODULE_TEST_SRC
        ${CMAKE_CURRENT_LIST_DIR}/workbench_project_commands_tests.cpp
    )

set(MODULE_TEST_LINK
    project
    au3wrap
    au3-project
    au3-project-file-io
    au3-tags
    au3-realtime-effects
    trackedit
    muse::actions
    muse::draw
    sqlite # required for SQLITE_CANTOPEN and SQLITE_READONLY macros used in aguproject_tests.cpp
)

set(MODULE_TEST_INCLUDE
    ${PROJECT_SOURCE_DIR}/src/project # for internal headers included relatively
)
set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)

    add_test(
        NAME workbench_project_commands
        COMMAND au_project_tests --gtest_filter=workbench_project_commands.*
    )

    add_test(
        NAME local_project_roundtrip
        COMMAND au_project_tests --gtest_filter=Project_AGuProjectTests.LocalFile_SaveAndReloadSucceeds
    )

    # The full suite runs this round-trip too; both tests use local_copy.aup4.
    set_tests_properties(au_project_tests local_project_roundtrip PROPERTIES RESOURCE_LOCK agu_project_test_data)
