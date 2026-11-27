import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import AGu.UiComponents
import "VolumeSliderMath.js" as SliderMath

StyledSlider {
    id: root

    Layout.fillWidth: true

    property double snapPoint: 0.0
    property double snapRange: 1.2
    property bool shiftPressed: false

    signal newVolumeRequested(real volume, bool completed)

    implicitHeight: 20

    background: Item {
        anchors.fill: parent
        NavigationFocusBorder {
            navigationCtrl: root.navigation
        }
        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            height: 2
            radius: 1
            color: ui.theme.fontSecondaryColor
            opacity: 0.4
        }
        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            width: root.handle.x + root.handle.width / 2
            height: 2
            radius: 1
            color: ui.theme.fontPrimaryColor
            opacity: 0.75
        }
        Rectangle {
            x: root.leftPadding + (root.snapPoint - root.from) / (root.to - root.from) * (root.availableWidth - root.handle.width) + root.handle.width / 2
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: 6
            color: ui.theme.fontPrimaryColor
            opacity: 0.8
        }
    }

    handle: Rectangle {
        x: root.leftPadding + root.visualPosition * (root.availableWidth - width)
        y: root.topPadding + root.availableHeight / 2 - height / 2
        width: 8
        height: 12
        radius: 3
        color: ui.theme.fontPrimaryColor
    }

    from: -60.0
    to: 12.0

    onValueChanged: {
        newVolumeRequested(value, false)
    }

    QtObject {
        id: prv

        property bool dragActive: false
        property real handleWidth: root.handle ? root.handle.width : 0
        property real startPos: 0.0
        property real startFineValue: 0.0
        property real handleOffset: 0.0
    }

    Connections {
        target: Qt.application

        function onStateChanged() {
            if (Qt.application.state !== Qt.ApplicationActive) {
                prv.dragActive = false
                tooltip.hide(true)
            }
        }
    }

    ValueTooltip {
        id: tooltip

        parent: root.handle

        unitText: "dB"
        sizingText: "-60.0dB"
        value: root.value
    }

    // We have to reimplement dragging to allow the tooltip
    // to stay on when the mouse is moved outside of the component
    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onPressed: {
            prv.dragActive = true
            const center = root.handle.x + root.handle.width / 2
            const overHandle = mouseX >= root.handle.x && mouseX <= root.handle.x + root.handle.width
            prv.handleOffset = overHandle ? mouseX - center : 0
            root.shiftPressed = (mouse.modifiers & Qt.ShiftModifier) !== 0
            if (!overHandle) {
                root.value = sliderValue()
            }
            tooltip.show(true)
            prv.startPos = mouseX
            prv.startFineValue = root.value
        }

        onDoubleClicked: {
            root.newVolumeRequested(root.value = 0, true)
        }

        onReleased: {
            prv.dragActive = false
            root.shiftPressed = false
            if (!containsMouse) {
                tooltip.hide(true)
            }
            root.newVolumeRequested(root.value, true)
        }

        onEntered: {
            tooltip.show()
        }

        onExited: {
            if (!prv.dragActive) {
                tooltip.hide(true)
            }
        }

        onPositionChanged: function (e) {
            if (!prv.dragActive) {
                return
            }

            if ((e.modifiers & (Qt.ShiftModifier))) {
                if (!root.shiftPressed) {
                    root.shiftPressed = true
                    prv.startPos = mouseX
                    prv.startFineValue = root.value
                }

                root.value = fineSliderValue()
            } else {
                if (root.shiftPressed) {
                    root.shiftPressed = false
                    prv.startPos = mouseX
                    prv.startFineValue = root.value
                    prv.handleOffset = mouseX - root.handle.x - root.handle.width / 2
                }
                root.value = sliderValue()
            }
        }

        function sliderValue() {
            const value = SliderMath.volumeAtPointer(mouseX, width, prv.handleWidth, prv.handleOffset, root.from, root.to)
            return SliderMath.snapVolume(value, root.shiftPressed, root.snapRange)
        }

        function fineSliderValue() {
            let step = 2 * (mouseX - prv.startPos) / (root.to - root.from)
            return SliderMath.clampVolume(prv.startFineValue + step, root.from, root.to)
        }
    }
}
