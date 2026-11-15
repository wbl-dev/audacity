muse_create_qml_module(appshell_qml FOR appshell)

qt_add_qml_module(appshell_qml
    URI Audacity.AppShell
    VERSION 1.0
    SOURCES
        aboutmodel.cpp
        aboutmodel.h
        appmenumodel.cpp
        appmenumodel.h
        FirstLaunchSetup/clipvisualizationpagemodel.cpp
        FirstLaunchSetup/clipvisualizationpagemodel.h
        FirstLaunchSetup/firstlaunchsetupmodel.cpp
        FirstLaunchSetup/firstlaunchsetupmodel.h
        FirstLaunchSetup/themespagemodel.cpp
        FirstLaunchSetup/themespagemodel.h
        framelesswindowmodel.cpp
        framelesswindowmodel.h
        maintoolbarmodel.cpp
        maintoolbarmodel.h
        mainwindowtitleprovider.cpp
        mainwindowtitleprovider.h
        projectpagemodel.cpp
        projectpagemodel.h
        windowdroparea.cpp
        windowdroparea.h
    QML_FILES
        AboutDialog.qml
        AboutDialogAudacityTab.qml
        AboutDialogPrivacyTab.qml
        AppWindow.qml
        FirstLaunchSetup/ClipVisualizationPage.qml
        FirstLaunchSetup/DoublePage.qml
        FirstLaunchSetup/FirstLaunchSetupDialog.qml
        FirstLaunchSetup/Page.qml
        FirstLaunchSetup/ThemesPage.qml
        HomePage/HomeMenu.qml
        HomePage/HomePage.qml
        Main.qml
        MainToolBar.qml
        platform/AppButtonBackground.qml
        platform/AppMenuBar.qml
        platform/PlatformMenuBar.qml
        ProjectPage/ProjectPage.qml
        ProjectPage/ProjectStatusBar.qml
        shared/AccentColorsList.qml
        shared/ThemeSamplesList.qml
        shared/internal/ThemeSample.qml
        WindowContent.qml
    RESOURCES
        FirstLaunchSetup/resources/ClipVisuals_ClassicClips.png
        FirstLaunchSetup/resources/ClipVisuals_ColorfulClips.png
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
        TARGET muse_dockwindow_qml
        TARGET muse_diagnostics_qml
        TARGET muse_shortcuts_qml
        TARGET muse_toast_qml
)

if (OS_IS_MAC)
    qt_target_qml_sources(appshell_qml
        QML_FILES
            platform/mac/Main.qml
    )
elseif(OS_IS_WIN)
    qt_target_qml_sources(appshell_qml
        QML_FILES
            platform/win/AppSystemButtons.qml
            platform/win/AppTitleBar.qml
            platform/win/Main.qml
    )
elseif(OS_IS_LIN OR OS_IS_FREEBSD)
    qt_target_qml_sources(appshell_qml
        QML_FILES
            platform/linux/Main.qml
    )
else()
    message(FATAL_ERROR "Unsupported OS")
endif()

if (NOT OS_IS_MAC)
    target_sources(appshell_qml PRIVATE
        navigableappmenumodel.cpp
        navigableappmenumodel.h
    )

    # we use QKeyMapper to fix https://github.com/musescore/MuseScore/issues/10181
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
    target_link_libraries(appshell_qml PRIVATE Qt6::GuiPrivate)
endif(NOT OS_IS_MAC)

target_link_libraries(appshell_qml PRIVATE Qt::Widgets Qt::Quick)

fixup_qml_module_dependencies(appshell_qml)

# Necessary for the auto-generated sources
target_include_directories(appshell_qml PRIVATE
    FirstLaunchSetup
)
