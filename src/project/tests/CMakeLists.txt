#
# Audacity: A Digital Audio Editor
#

set(MODULE_TEST au_project_tests)

set(MODULE_TEST_SRC
    ${CMAKE_CURRENT_LIST_DIR}/environment.cpp

    ${CMAKE_CURRENT_LIST_DIR}/mocks/audacityprojectmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/projectconfigurationmock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/trackeditprojectcreatormock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/cloudprojectsprovidermock.h
    ${CMAKE_CURRENT_LIST_DIR}/mocks/dummyeffectinstancefactory.h
    ${CMAKE_CURRENT_LIST_DIR}/audacityproject_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/projectrecovery_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/realtimeeffectspersistence_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cliptempopersistence_tests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/au3metadata_tests.cpp
)

if(AU4_WORKBENCH_PROFILE)
    list(APPEND MODULE_TEST_SRC
        ${CMAKE_CURRENT_LIST_DIR}/workbench_project_commands_tests.cpp
    )
endif()

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
    sqlite # required for SQLITE_CANTOPEN and SQLITE_READONLY macros used in audacityproject_tests.cpp
)

set(MODULE_TEST_INCLUDE
    ${PROJECT_SOURCE_DIR}/src/project # for internal headers included relatively
)
set(MODULE_TEST_DATA_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(SetupGTest)

if(AU4_WORKBENCH_PROFILE)
    add_test(
        NAME workbench_project_commands
        COMMAND au_project_tests --gtest_filter=workbench_project_commands.*
    )

    add_test(
        NAME workbench_local_cloud_metadata
        COMMAND au_project_tests --gtest_filter=Project_Audacity4ProjectTests.Load_LocalFileWithExternalCloudRecord_ReturnsSuccessWithoutCloudOperation
    )
endif()
