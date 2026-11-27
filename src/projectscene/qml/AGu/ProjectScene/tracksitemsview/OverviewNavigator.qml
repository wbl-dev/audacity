import QtQuick

import Muse.Ui
import Muse.UiComponents
import AGu.ProjectScene
import "OverviewNavigatorMath.js" as NavigatorMath

Rectangle {
    id: root

    required property TimelineContext context
    property real playPosition: 0
    property real savedViewStart: -1
    property real savedViewEnd: -1

    color: ui.theme.backgroundSecondaryColor

    Rectangle {
        id: strip
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 0
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

        readonly property real duration: Math.max(1, overview.duration)
        readonly property real viewLeft: Math.max(0, Math.min(width, root.context.frameStartTime / duration * width))
        readonly property real viewRight: Math.max(viewLeft, Math.min(width, root.context.frameEndTime / duration * width))

        Rectangle {
            visible: root.context.selectionActive
            x: Math.max(0, Math.min(parent.width, root.context.selectionStartTime / strip.duration * parent.width))
            width: Math.max(1, Math.min(parent.width, root.context.selectionEndTime / strip.duration * parent.width) - x)
            height: parent.height
            color: ui.theme.extra["selection_highlight_color"]
            opacity: 0.25
        }

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
            cursorShape: pressed ? (edge === 0 ? Qt.ClosedHandCursor : Qt.SizeHorCursor) : (NavigatorMath.resizeEdgeAt(mouseX, strip.viewLeft, strip.viewRight) === 0 ? Qt.OpenHandCursor : Qt.SizeHorCursor)

            property int edge: 0
            property real pressTime: 0
            property real initialStart: 0
            property real initialEnd: 0

            onPressed: mouse => {
                initialStart = root.context.frameStartTime
                initialEnd = Math.min(strip.duration, root.context.frameEndTime)
                pressTime = mouse.x / width * strip.duration
                edge = NavigatorMath.resizeEdgeAt(mouse.x, strip.viewLeft, strip.viewRight)
                if (edge === 0 && (mouse.x < strip.viewLeft || mouse.x > strip.viewRight)) {
                    const span = initialEnd - initialStart
                    root.context.setViewRange(pressTime - span / 2, pressTime + span / 2)
                    initialStart = root.context.frameStartTime
                    initialEnd = root.context.frameEndTime
                }
            }
            onDoubleClicked: mouse => {
                if (mouse.x < strip.viewLeft || mouse.x > strip.viewRight) {
                    return
                }
                const currentlyFit = Math.abs(root.context.frameStartTime) < 0.001 && Math.abs(root.context.frameEndTime - strip.duration) < 0.001
                if (currentlyFit && root.savedViewStart >= 0 && root.savedViewEnd > root.savedViewStart) {
                    root.context.setViewRange(root.savedViewStart, root.savedViewEnd)
                } else {
                    root.savedViewStart = root.context.frameStartTime
                    root.savedViewEnd = root.context.frameEndTime
                    root.context.setViewRange(0, strip.duration)
                }
            }
            onWheel: wheel => {
                const deltaX = wheel.pixelDelta.x !== 0 ? wheel.pixelDelta.x : wheel.angleDelta.x / 4
                const deltaY = wheel.pixelDelta.y !== 0 ? wheel.pixelDelta.y : wheel.angleDelta.y / 4
                const span = root.context.frameEndTime - root.context.frameStartTime
                if (Math.abs(deltaX) > Math.abs(deltaY)) {
                    const shift = -deltaX * span / strip.width
                    root.context.setViewRange(root.context.frameStartTime + shift, root.context.frameEndTime + shift)
                } else if (deltaY !== 0) {
                    const nextSpan = Math.max(0.001, Math.min(strip.duration, span * Math.exp(-deltaY * 0.003)))
                    const anchor = wheel.x / strip.width
                    const anchorTime = root.context.frameStartTime + span * anchor
                    root.context.setViewRange(anchorTime - nextSpan * anchor, anchorTime + nextSpan * (1 - anchor))
                }
                wheel.accepted = true
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
