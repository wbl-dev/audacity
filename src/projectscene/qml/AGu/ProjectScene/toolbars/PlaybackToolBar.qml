/*
* Audacity: A Digital Audio Editor
*/
import QtQuick

import Muse.Ui
import Muse.UiComponents

import AGu.ProjectScene
import AGu.UiComponents

Item {
    id: root

    property bool floating: false
    property int maximumWidth: 0
    property int maximumHeight: 0

    property alias navigationPanel: view.navigationPanel

    width: view.width
    height: view.height

    signal relayoutRequested

    StyledToolBarView {
        id: view

        navigationPanel.name: "PlaybackToolBar"
        navigationPanel.accessible.name: qsTrc("projectscene", "Transport and editing toolbar")

        anchors.verticalCenter: parent.verticalCenter

        spacing: 4
        rowHeight: 56
        leftPadding: 16
        rightPadding: 16
        useSpacersAsSeparators: true
        spacerWidth: 12
        maximumWidth: root.maximumWidth > 0 ? Math.max(0, root.maximumWidth - leftPadding - rightPadding) : -1

        model: PlaybackToolBarModel {
            onItemsChanged: root.relayoutRequested()
        }

        sourceComponentCallback: function (type) {
            switch (type) {
            case PlaybackToolBarModel.PLAYBACK_CONTROL:
                return controlComp
            case PlaybackToolBarModel.PLAYBACK_LEVEL:
                return masterVolumeComp
            case PlaybackToolBarModel.PLAYBACK_TIME:
                return playbackTimeComp
            case PlaybackToolBarModel.PROJECT_CONTROL:
                return projectControlComp
            case PlaybackToolBarModel.SNAP:
                return snapComp
            }
            return null
        }

        Component {
            id: controlComp

            StyledToolBarItem {
                id: controlButton

                width: 32
                height: 32
                backgroundRadius: 5
                iconFont: ui.theme.iconsFont
                iconColor: itemData ? itemData.iconColor : ui.theme.fontPrimaryColor
                accentColor: itemData ? itemData.backgroundColor : ui.theme.buttonColor
                accentButton: itemData ? itemData.selected : false

                backgroundItem: Rectangle {
                    radius: controlButton.backgroundRadius
                    color: controlButton.accentButton ? controlButton.accentColor : controlButton.mouseArea.containsMouse || controlButton.mouseArea.pressed ? ui.theme.buttonColor : "transparent"

                    NavigationFocusBorder {
                        navigationCtrl: controlButton.navigation
                    }
                }
            }
        }

        Component {
            id: projectControlComp

            StyledToolBarItem {
                id: editButton

                width: 28
                height: 32
                iconFont: ui.theme.iconsFont

                backgroundItem: Rectangle {
                    radius: 5
                    color: editButton.mouseArea.containsMouse || editButton.mouseArea.pressed ? ui.theme.buttonColor : "transparent"

                    NavigationFocusBorder {
                        navigationCtrl: editButton.navigation
                    }
                }
            }
        }

        Component {
            id: playbackTimeComp

            Timecode {
                property var itemData: null

                accessibleName: qsTrc("projectscene", "Playback position:")
                appearance: Timecode.Appearance.Embedded
                textColor: ui.theme.fontPrimaryColor
                value: itemData ? itemData.currentValue : 0
                mode: TimecodeModeSelector.TimePoint
                sampleRate: itemData ? itemData.sampleRate : 0
                tempo: itemData ? itemData.tempo : 0
                upperTimeSignature: itemData ? itemData.upperTimeSignature : -1
                lowerTimeSignature: itemData ? itemData.lowerTimeSignature : -1
                currentFormat: itemData ? itemData.currentFormat : -1
                enabled: itemData ? itemData.enabled : false

                onValueChangeRequested: function (newValue) {
                    if (itemData) {
                        itemData.currentValue = newValue
                    }
                }

                onCurrentFormatChanged: {
                    if (itemData) {
                        itemData.currentFormat = currentFormat
                    }
                }
            }
        }

        Component {
            id: snapComp

            DropdownWithTitle {
                property var itemData: null

                width: 132
                height: 28
                //: Noun: label of the snapping control in the playback toolbar
                title: qsTrc("projectscene", "Snap")
                toggleAccessibleName: qsTrc("projectscene", "Snapping")
                dropdownAccessibleName: qsTrc("projectscene", "Snap to")
                current: itemData ? itemData.currentValue : ""
                model: itemData ? itemData.availableSnapTypes : null
                isOptionEnabled: itemData ? itemData.isSnapEnabled : false
                enabled: itemData ? itemData.enabled : false

                onIsOptionEnableChangeRequested: function (enabled) {
                    if (itemData) {
                        itemData.isSnapEnabled = enabled
                    }
                }

                onHandleMenuItem: function (itemId) {
                    if (itemData) {
                        itemData.handleMenuItem(itemId)
                    }
                }
            }
        }

        Component {
            id: masterVolumeComp

            Item {
                id: masterVolume

                property var itemData: null
                property alias navigation: masterSlider.navigation

                width: view.maximumWidth > 0 && view.maximumWidth < 980 ? 140 : 180
                height: 32
                enabled: itemData ? itemData.enabled : false
                opacity: enabled ? 1.0 : ui.theme.itemOpacityDisabled

                StyledIconLabel {
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    width: 20
                    height: 20
                    iconCode: IconCode.AUDIO
                    color: ui.theme.fontSecondaryColor
                }

                StyledSlider {
                    id: masterSlider

                    anchors.left: parent.left
                    anchors.leftMargin: 28
                    anchors.right: parent.right
                    anchors.top: parent.top
                    height: 22

                    from: -60
                    to: 0
                    stepSize: 0.5
                    value: masterVolume.itemData ? Math.max(from, masterVolume.itemData.level) : 0

                    navigation.name: "MasterVolume"
                    navigation.accessible.name: qsTrc("playback", "Master volume")

                    onMoved: {
                        if (masterVolume.itemData) {
                            masterVolume.itemData.level = value <= from ? -145 : value
                        }
                    }

                    onHoveredChanged: hovered ? volumeTooltip.show() : volumeTooltip.hide()
                    onPressedChanged: pressed ? volumeTooltip.show(true) : volumeTooltip.hide(true)

                    background: Item {
                        anchors.fill: parent

                        NavigationFocusBorder {
                            navigationCtrl: masterSlider.navigation
                        }

                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width
                            height: 2
                            radius: 1
                            color: ui.theme.fontSecondaryColor
                            opacity: 0.35
                        }

                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: masterSlider.handle.x + masterSlider.handle.width / 2
                            height: 2
                            radius: 1
                            color: ui.theme.fontPrimaryColor
                            opacity: 0.75
                        }
                    }

                    handle: Rectangle {
                        x: masterSlider.leftPadding + masterSlider.visualPosition * (masterSlider.availableWidth - width)
                        y: masterSlider.topPadding + masterSlider.availableHeight / 2 - height / 2
                        width: 8
                        height: 12
                        radius: 3
                        color: ui.theme.fontPrimaryColor
                    }

                    ValueTooltip {
                        id: volumeTooltip

                        parent: masterSlider.handle
                        unitText: "dB"
                        sizingText: "-60.0dB"
                        value: masterSlider.value
                    }
                }

                Column {
                    anchors.left: masterSlider.left
                    anchors.right: masterSlider.right
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 3
                    spacing: 2

                    Repeater {
                        model: 2

                        Rectangle {
                            required property int index
                            readonly property real pressure: !masterVolume.itemData || !masterVolume.itemData.isPlaying ? -145 : index === 0 ? masterVolume.itemData.leftChannelPressure : masterVolume.itemData.rightChannelPressure

                            width: parent.width
                            height: 2
                            radius: 1
                            color: ui.theme.backgroundQuarternaryColor

                            Rectangle {
                                width: parent.width * Math.max(0, Math.min(1, (parent.pressure + 60) / 60))
                                height: parent.height
                                radius: 1
                                color: parent.pressure >= 0 ? "#e27070" : ui.theme.accentColor
                                opacity: 0.75
                            }
                        }
                    }
                }
            }
        }
    }
}
