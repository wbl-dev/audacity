/*
 * Audacity: A Digital Audio Editor
 */
import QtQuick 2.15
import QtQuick.Layouts 1.3

import Muse.Ui 1.0
import Muse.UiComponents
import AGu.Project 1.0

import "internal/ProjectsPage"

FocusScope {
    id: root

    readonly property int sideMargin: 46

    NavigationSection {
        id: navSec
        name: "RecentProjects"
        enabled: root.enabled && root.visible
        order: 3
    }

    ProjectsPageModel {
        id: projectsPageModel
    }

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundTertiaryColor
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: root.sideMargin
        anchors.rightMargin: root.sideMargin
        anchors.topMargin: root.sideMargin

        spacing: 20

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 12

            StyledTextLabel {
                text: qsTrc("project", "Welcome to AGu")
                font: ui.theme.titleBoldFont
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                FlatButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 76
                    icon: IconCode.NEW_FILE
                    text: qsTrc("project", "New blank project")
                    textFont: ui.theme.largeBodyBoldFont
                    orientation: Qt.Horizontal
                    accentButton: true
                    backgroundRadius: 8
                    onClicked: projectsPageModel.createNewProject()
                }

                FlatButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 76
                    icon: IconCode.OPEN_FILE
                    text: qsTrc("project", "Open audio file…")
                    textFont: ui.theme.largeBodyBoldFont
                    orientation: Qt.Horizontal
                    backgroundRadius: 8
                    onClicked: Qt.callLater(projectsPageModel.openOther)
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            StyledTextLabel {
                Layout.fillWidth: true
                text: qsTrc("project", "Projects")
                font: ui.theme.titleBoldFont
                horizontalAlignment: Text.AlignLeft
            }

            SearchField {
                id: searchField
                Layout.preferredWidth: 220
                accessible.name: qsTrc("project", "Search recent projects")
            }
        }

        RadioButtonGroup {
            id: viewTypeRadioButtons
            Layout.alignment: Qt.AlignRight

            model: [
                {
                    "icon": IconCode.GRID,
                    "title": qsTrc("project", "Grid view"),
                    "value": ProjectsPageModel.Grid
                },
                {
                    "icon": IconCode.LIST,
                    "title": qsTrc("project", "List view"),
                    "value": ProjectsPageModel.List
                }
            ]

            delegate: FlatRadioButton {
                implicitWidth: ui.theme.defaultButtonSize
                implicitHeight: ui.theme.defaultButtonSize
                checked: projectsPageModel.viewType === modelData.value
                iconCode: modelData.icon
                transparent: true
                checkedColor: ui.theme.buttonColor

                onToggled: {
                    projectsPageModel.viewType = modelData.value
                }
            }
        }

        RecentProjectsView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            viewType: projectsPageModel.viewType
            searchText: searchField.searchText
            backgroundColor: ui.theme.backgroundTertiaryColor
            sideMargin: 0
            navigationSection: navSec
            navigationOrder: 4

            onCreateNewProjectRequested: projectsPageModel.createNewProject()
            onOpenProjectRequested: function (projectPath, displayName) {
                Qt.callLater(projectsPageModel.openProject, projectPath, displayName)
            }
        }
    }
}
