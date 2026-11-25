/*
* Audacity: A Digital Audio Editor
*/
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import AGu.ProjectScene

import "audio"

TrackItem {
    id: root

    extraControlsComponent: Component {
        ColumnLayout {
            spacing: 7

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Loader {
                    sourceComponent: trackControlButtons
                }

                Item {
                    Layout.fillWidth: true
                }

                FlatToggleButton {
                    Layout.preferredWidth: 28
                    Layout.preferredHeight: 24
                    icon: root.trackViewState.isMultiView ? IconCode.ARROW_UP : IconCode.ARROW_DOWN
                    checked: root.trackViewState.isMultiView
                    toolTipDescription: qsTrc("projectscene", "Show waveform and spectrogram")
                    navigation.panel: root.headerNavigationPanel
                    navigation.order: root.extraControlsNavigationStart + 2
                    navigation.accessible.name: qsTrc("projectscene", "Waveform and spectrogram")
                    onToggled: root.trackViewState.toggleMultiView()
                }
            }

            VolumeSlider {
                id: volumeSlider
                Layout.fillWidth: true
                Layout.preferredHeight: 20
                value: Boolean(root.item) ? root.item.volumeLevel : 0
                navigation.panel: root.headerNavigationPanel
                navigation.order: root.extraControlsNavigationStart + 3
                navigation.accessible.name: qsTrc("projectscene", "Track volume")
                onNewVolumeRequested: function (newValue, completed) {
                    if (Boolean(root.item)) {
                        root.item.setVolumeLevel(newValue, completed)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                visible: !root.collapsed && root.height >= 160
                spacing: 12

                StyledTextLabel {
                    text: qsTrc("projectscene", "Pan")
                }

                PanKnob {
                    id: panKnob
                    value: Boolean(root.item) ? root.item.pan : 0
                    navigation.panel: root.headerNavigationPanel
                    navigation.order: volumeSlider.navigation.order + 1
                    navigation.accessible.name: qsTrc("projectscene", "Pan")
                    onNewPanRequested: function (newValue, completed) {
                        if (Boolean(root.item)) {
                            root.item.setPan(newValue, completed)
                        }
                    }
                }

                FlatButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 28
                    text: qsTrc("projectscene", "Effects")
                    toolTipDescription: qsTrc("projectscene", "Opens effects panel")
                    navigation.panel: root.headerNavigationPanel
                    navigation.order: panKnob.navigation.order + 1
                    onClicked: root.openEffectsRequested()
                }
            }

            Component.onCompleted: {
                root.extraControlsNavigationEnd = Qt.binding(function () {
                    return volumeSlider.navigation.order + 2
                })
            }
        }
    }

    rightSideContainerComponent: Component {
        Item {
            width: parent.width
            height: parent.height

            Row {
                id: volumePressureContainer

                property int indicatorWidth: 7

                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 2

                function clearMeters() {
                    leftOrMonoVolumePressureMeter.reset()
                    leftOrMonoVolumePressureMeter.resetClipped()
                    rightVolumePressureMeter.reset()
                    rightVolumePressureMeter.resetClipped()
                }

                TapHandler {
                    onTapped: volumePressureContainer.clearMeters()
                }

                VolumePressureMeter {
                    id: leftOrMonoVolumePressureMeter
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    indicatorWidth: parent.indicatorWidth
                    meterModel: trackViewState.meterModel
                    currentVolumePressure: Boolean(root.item) ? root.item.leftChannelPressure : 0
                    currentRMS: Boolean(root.item) ? root.item.leftChannelRMS : 0
                }

                VolumePressureMeter {
                    id: rightVolumePressureMeter
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    indicatorWidth: parent.indicatorWidth
                    meterModel: trackViewState.meterModel
                    currentVolumePressure: Boolean(root.item) ? root.item.rightChannelPressure : 0
                    currentRMS: Boolean(root.item) ? root.item.rightChannelRMS : 0
                }

                Connections {
                    target: root.trackViewState

                    function onIsRecordingChanged() {
                        if (trackViewState.isRecording) {
                            leftOrMonoVolumePressureMeter.resetClipped()
                            rightVolumePressureMeter.resetClipped()
                        }

                        leftOrMonoVolumePressureMeter.reset()
                        rightVolumePressureMeter.reset()
                    }

                    function onIsPlayingChanged() {
                        if (trackViewState.isPlaying) {
                            volumePressureContainer.clearMeters()
                        }
                    }
                }

                Connections {
                    target: root

                    function onIsFocusedChanged() {
                        if (!root.isFocused) {
                            volumePressureContainer.clearMeters()
                        }
                    }
                }
            }

            states: [
                State {
                    when: Boolean(root.item) && root.item.channelCount === 1
                    name: "mono"
                    PropertyChanges {
                        target: volumePressureContainer
                        indicatorWidth: 8
                    }
                    PropertyChanges {
                        target: rightVolumePressureMeter
                        visible: false
                    }
                },
                State {
                    when: Boolean(root.item) && root.item.channelCount === 2
                    name: "stereo"
                    PropertyChanges {
                        target: volumePressureContainer
                        indicatorWidth: 7
                    }
                    PropertyChanges {
                        target: rightVolumePressureMeter
                        visible: true
                    }
                }
            ]
        }
    }

    Component {
        id: trackControlButtons

        RowLayout {
            spacing: 4

            FlatToggleButton {
                id: muteButton

                Layout.preferredWidth: 36
                Layout.preferredHeight: 24

                StyledTextLabel {
                    anchors.centerIn: parent
                    text: qsTrc("projectscene", "M")
                }
                checked: Boolean(root.item) ? root.item.muted : false

                navigation.panel: root.headerNavigationPanel
                navigation.order: root.extraControlsNavigationStart
                //: Accessibility name of the mute button
                navigation.accessible.name: qsTrc("projectscene", "Mute")
                navigation.accessible.role: MUAccessible.CheckBox

                onToggled: {
                    if (Boolean(root.item)) {
                        root.item.muted = !checked
                    }
                }
            }

            FlatToggleButton {
                id: soloButton

                Layout.preferredWidth: 36
                Layout.preferredHeight: 24

                StyledTextLabel {
                    anchors.centerIn: parent
                    text: qsTrc("projectscene", "S")
                }
                checked: Boolean(root.item) ? root.item.solo : false

                navigation.panel: root.headerNavigationPanel
                navigation.order: muteButton.navigation.order + 1
                //: Accessibility name of the solo button
                navigation.accessible.name: qsTrc("projectscene", "Solo")
                navigation.accessible.role: MUAccessible.CheckBox

                onToggled: {
                    if (Boolean(root.item)) {
                        root.item.solo = !checked
                    }
                }

                Component.onCompleted: {
                    root.headerTrailingControlsNavigationEnd = Qt.binding(function () {
                        return soloButton.navigation.order
                    })
                }
            }
        }
    }
}
