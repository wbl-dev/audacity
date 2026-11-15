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

        RowLayout {
            Layout.fillWidth: true

            StyledTextLabel {
                Layout.fillWidth: true
                text: qsTrc("project", "Projects")
                font: ui.theme.titleBoldFont
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
            sideMargin: root.sideMargin
            navigationSection: navSec
            navigationOrder: 4

            onCreateNewProjectRequested: projectsPageModel.createNewProject()
            onOpenProjectRequested: function (projectPath, displayName) {
                Qt.callLater(projectsPageModel.openProject, projectPath, displayName)
            }
        }

        SeparatorLine {
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 10

            Item {
                Layout.fillWidth: true
            }

            FlatButton {
                text: qsTrc("project", "New")
                onClicked: projectsPageModel.createNewProject()
            }

            FlatButton {
                text: qsTrc("project", "Open other…")
                onClicked: Qt.callLater(projectsPageModel.openOther)
            }
        }
    }
}
