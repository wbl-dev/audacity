import QtQuick

import Muse.Ui
import Muse.UiComponents
import AGu.ProjectScene

Rectangle {
    id: root

    required property TimelineContext context
    property real playPosition: 0

    color: ui.theme.backgroundSecondaryColor

    Rectangle {
        id: strip
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 8
        anchors.bottomMargin: 8
        color: ui.theme.backgroundPrimaryColor
        border.width: 1
        border.color: ui.theme.strokeColor
        clip: true

        ProjectOverview {
            id: overview
            anchors.fill: parent
            anchors.margins: 2
            Component.onCompleted: init()
        }

        readonly property real duration: overview.duration
        readonly property real viewLeft: Math.max(0, Math.min(width, root.context.frameStartTime / duration * width))
        readonly property real viewRight: Math.max(viewLeft, Math.min(width, root.context.frameEndTime / duration * width))

        Rectangle {
            width: strip.viewLeft
            height: parent.height
            color: "#80000000"
        }
        Rectangle {
            x: strip.viewRight
            width: parent.width - x
            height: parent.height
            color: "#80000000"
        }
        Rectangle {
            x: strip.viewLeft
            width: Math.max(2, strip.viewRight - x)
            height: parent.height
            color: "transparent"
            border.color: ui.theme.fontSecondaryColor
            border.width: 1

            Rectangle {
                width: 3
                height: 12
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                color: ui.theme.fontSecondaryColor
            }
            Rectangle {
                width: 3
                height: 12
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                color: ui.theme.fontSecondaryColor
            }
        }

        Rectangle {
            x: root.playPosition / strip.duration * parent.width
            width: 1
            height: parent.height
            color: ui.theme.accentColor
        }

        MouseArea {
            id: navigator
            anchors.fill: parent
            hoverEnabled: true
            preventStealing: true
            cursorShape: pressed ? (edge === 0 ? Qt.ClosedHandCursor : Qt.SizeHorCursor) : (Math.abs(mouseX - strip.viewLeft) < 7 || Math.abs(mouseX - strip.viewRight) < 7 ? Qt.SizeHorCursor : Qt.OpenHandCursor)

            property int edge: 0
            property real pressTime: 0
            property real initialStart: 0
            property real initialEnd: 0

            onPressed: mouse => {
                initialStart = root.context.frameStartTime
                initialEnd = Math.min(strip.duration, root.context.frameEndTime)
                pressTime = mouse.x / width * strip.duration
                edge = Math.abs(mouse.x - strip.viewLeft) < 7 ? -1 : (Math.abs(mouse.x - strip.viewRight) < 7 ? 1 : 0)
                if (edge === 0 && (mouse.x < strip.viewLeft || mouse.x > strip.viewRight)) {
                    const span = initialEnd - initialStart
                    root.context.setViewRange(pressTime - span / 2, pressTime + span / 2)
                    initialStart = root.context.frameStartTime
                    initialEnd = root.context.frameEndTime
                }
            }
            onPositionChanged: mouse => {
                if (!pressed) {
                    return
                }
                const time = Math.max(0, Math.min(strip.duration, mouse.x / width * strip.duration))
                const minimumSpan = Math.max(0.001, strip.duration * 12 / width)
                if (edge < 0) {
                    root.context.setViewRange(Math.min(time, initialEnd - minimumSpan), initialEnd)
                } else if (edge > 0) {
                    root.context.setViewRange(initialStart, Math.max(time, initialStart + minimumSpan))
                } else {
                    root.context.setViewRange(initialStart + time - pressTime, initialEnd + time - pressTime)
                }
            }
        }
    }

    SeparatorLine {
        anchors.bottom: parent.bottom
    }
}
