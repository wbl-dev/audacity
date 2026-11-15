/*
 * Audacity: A Digital Audio Editor
 */
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents
import Muse.GraphicalEffects

import AGu.UiComponents 1.0
import AGu.ProjectScene

BaseSection {
    id: root

    title: qsTrc("preferences", "Asymmetric stereo heights")

    navigation.direction: NavigationPanel.Both

    property var editPreferencesModel: null
    navigationOrderEnd: navigation.order

    rowSpacing: 16

    Rectangle {
        id: svgImg

        width: 220
        height: 64

        layer.enabled: true
        layer.effect: RoundedCornersEffect {
            radius: 5
        }

        Image {
            source: "../resources/AsymmetricStereoHeights.svg"

            width: svgImg.width
            height: svgImg.height

            Image {
                source: "../resources/StereoHeightsCursor.png"

                width: 19
                height: 21

                x: 68
                y: 27
            }
        }
    }

    StyledTextLabel {
        text: qsTrc("preferences", "Dragging on the center line may adjust the height of the channel:")
    }

    RadioButtonGroup {

        width: parent.width
        height: 80

        spacing: root.rowSpacing
        orientation: Qt.Vertical

        Column {
            width: parent.width
            spacing: root.columnSpacing

            RoundedRadioButton {

                checked: editPreferencesModel.stereoHeightsPref === AsymmetricStereoHeights.ALWAYS
                text: qsTrc("preferences", "Always")

                navigation.name: "AlwaysRadioBtn"
                navigation.panel: root.navigation
                navigation.row: 0

                onToggled: {
                    editPreferencesModel.setStereoHeightsPref(AsymmetricStereoHeights.ALWAYS)
                }
            }

            RoundedRadioButton {
                checked: editPreferencesModel.stereoHeightsPref === AsymmetricStereoHeights.NEVER
                text: qsTrc("preferences", "Never")

                navigation.name: "NeverRadioBtn"
                navigation.panel: root.navigation
                navigation.row: 1

                onToggled: {
                    editPreferencesModel.setStereoHeightsPref(AsymmetricStereoHeights.NEVER)
                }
            }
        }
    }
}
