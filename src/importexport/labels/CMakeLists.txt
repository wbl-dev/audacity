#
# Audacity: A Digital Audio Editor
#

declare_module(iex_labels)

set(MODULE_QRC labels.qrc)

set(MODULE_SRC
    # public
    ${CMAKE_CURRENT_LIST_DIR}/labelsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/labelsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/ilabelsimporter.h
    ${CMAKE_CURRENT_LIST_DIR}/ilabelsexporter.h
    ${CMAKE_CURRENT_LIST_DIR}/ilabelsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/labelsutils.h
    ${CMAKE_CURRENT_LIST_DIR}/labelstypes.h
    ${CMAKE_CURRENT_LIST_DIR}/labelparserprofile.h
    ${CMAKE_CURRENT_LIST_DIR}/labelscommands.h

    #internals
    ${CMAKE_CURRENT_LIST_DIR}/internal/au3/au3labelsimporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/au3/au3labelsimporter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/au3/au3labelsexporter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/au3/au3labelsexporter.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/labelsconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscommandsregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscommandsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscommandscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscommandscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscommandsstate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscommandsstate.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscriptdata.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscriptengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscriptengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscriptworker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelscriptworker.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelparserregistry.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelparserregistry.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelparserrescan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/labelparserrescan.h

    ${CMAKE_CURRENT_LIST_DIR}/view/exportlabelsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/exportlabelsmodel.h
)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_LINK
        au3wrap
        au3-files
        au3-label-track
        au3-track
        au3-import-export
)

set(MODULE_USE_UNITY OFF)

setup_module()

if (AU_BUILD_IMPORT_TESTS)
    add_executable(labelscriptworker
        ${CMAKE_CURRENT_LIST_DIR}/internal/labelscriptworkermain.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/labelscriptworker.cpp
    )
    target_link_libraries(labelscriptworker PRIVATE Qt6::Core Qt6::Qml)
    add_subdirectory(tests)
endif()
