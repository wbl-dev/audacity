/*
 * Audacity: A Digital Audio Editor
 */
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents

import AGu.UiComponents 1.0
import AGu.Export 1.0

BaseSection {
    id: root

    property bool onlineDownloadsAvailable: true

    navigation.direction: NavigationPanel.Horizontal

    CustomFFmpegPreferencesModel {
        id: ffmpegPrefModel
    }

    Column {
        spacing: 12

        Row {
            StyledTextLabel {
                text: qsTrc("preferences", "FFmpeg library version:")
            }

            StyledTextLabel {
                text: ffmpegPrefModel.ffmpegVersion
            }
        }

        Row {
            spacing: 8

            TextInputField {
                id: formatField

                width: 223

                currentText: ffmpegPrefModel.ffmpegLibraryPath ?? qsTrc("preferences", "FFmpeg library not found")

                navigation.accessible.name: qsTrc("preferences", "FFmpeg library path")

                property var newLibPath

                onTextEditingFinished: function (newTextValue) {
                    let result = ffmpegPrefModel.setFFmpegLibraryPath(newTextValue)
                    currentText = ffmpegPrefModel.ffmpegLibraryPath
                    inputField.text = ffmpegPrefModel.ffmpegLibraryPath
                }
            }

            FlatButton {
                width: 178

                text: qsTrc("export", "Locate existing installation")

                navigation.panel: root.navigation
                navigation.order: root.onlineDownloadsAvailable ? downloadBtn.navigation.order + 1 : downloadBtn.navigation.order

                onClicked: {
                    ffmpegPrefModel.locateFFmpegLibrary()
                }
            }
        }
    }
}
