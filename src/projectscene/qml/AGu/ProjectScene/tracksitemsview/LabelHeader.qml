import QtQuick

import Muse.Ui
import Muse.UiComponents

Rectangle {
    id: root

    property string title: ""

    property bool isPoint: false
    property bool isSelected: false
    property bool compactRange: false
    property bool showTitle: true
    readonly property bool editing: titleLoader.isEditState
    readonly property bool hovered: headerDragArea.containsMouse
    property real pointAnchorX: 0
    property real pointAnchorY: 0
    property real timelineX: 0
    property real timelineWidth: 0

    property color backgroundColor: "transparent"
    property color labelColor: ui.theme.extra["black_color"]

    property int earWidth: 0

    component TimeSelection: QtObject {
        property real start: 0
        property real width: 0
        property color color: "transparent"
    }

    readonly property TimeSelection timeSelection: TimeSelection {}

    property real leftVisibleMargin: 0

    property bool enableCursorInteraction: true

    property var navigationPanel: null

    signal titleEditAccepted(var newTitle)

    signal editStarted
    signal editFinished

    signal requestSingleSelected

    signal mousePositionChanged(real x, real y)
    signal headerHoveredChanged(bool value)

    property int contentWidth: Math.max(50, Math.min(400, titleLoader.contentWidth + 8))
    property int contentHeight: titleLoader.isEditState && titleLoader.item ? titleLoader.item.contentHeight : 14

    x: root.isPoint ? Math.max(4, Math.min(root.pointAnchorX + 12, root.parent.width - width - 4)) : 0
    y: root.isPoint ? root.pointAnchorY : 0

    color: root.isPoint ? root.backgroundColor : "transparent"
    radius: 4
    border.width: root.isPoint && root.isSelected ? 1 : 0
    border.color: ui.theme.fontPrimaryColor

    function edit() {
        titleLoader.edit()
    }

    QtObject {
        id: prv

        readonly property int doubleClickInterval: 400
        readonly property int doubleClickMaxDistance: 5
    }

    Rectangle {
        id: rangeBar

        anchors.left: parent.left
        anchors.right: parent.right
        y: 5
        height: 12
        radius: 3
        color: root.backgroundColor
        border.width: root.isSelected ? 1 : 0
        border.color: ui.theme.fontPrimaryColor
        visible: !root.isPoint
    }

    Rectangle {
        id: selectionHighlight

        readonly property real startX: Math.max(0, root.timeSelection.start)
        readonly property real endX: Math.min(root.width, root.timeSelection.start + root.timeSelection.width)

        x: startX
        y: root.isPoint ? 0 : rangeBar.y
        width: Math.max(0, endX - startX)
        height: root.isPoint ? root.height : rangeBar.height

        radius: root.radius
        color: root.timeSelection.color
        visible: width > 0
    }

    MouseArea {
        id: headerDragArea
        anchors.fill: parent

        property var lastClickTime: Date()
        property point doubleClickStartPosition

        acceptedButtons: Qt.LeftButton
        hoverEnabled: true
        cursorShape: Qt.OpenHandCursor

        visible: root.enableCursorInteraction

        onPressed: function (e) {
            root.headerHoveredChanged(true)
            root.mousePositionChanged(e.x, e.y)
            var currentTime = Date.now()

            if (currentTime - lastClickTime < prv.doubleClickInterval) {
                // Double click - edit title
                titleLoader.edit()
            } else {
                // Single click - select
                root.requestSingleSelected()
                lastClickTime = currentTime
                doubleClickStartPosition = Qt.point(e.x, e.y)
            }
            e.accepted = false
        }

        onClicked: function (e) {
            e.accepted = false
        }

        onPositionChanged: function (e) {
            // Reset double click timer if the mouse has moved,
            // to prevent rapid clip movement activate title editing
            if (Math.abs(e.x - doubleClickStartPosition.x) > prv.doubleClickMaxDistance || Math.abs(e.y - doubleClickStartPosition.y) > prv.doubleClickMaxDistance) {
                lastClickTime = 0
            }

            root.mousePositionChanged(e.x, e.y)

            e.accepted = false
        }

        onContainsMouseChanged: {
            if (!root.visible) {
                return
            }
            root.headerHoveredChanged(containsMouse)
        }
    }

    Item {
        id: titleViewport

        visible: root.showTitle || root.editing

        anchors.top: parent.top
        anchors.topMargin: titleLoader.isEditState ? 0 : root.isPoint ? 2 : 4
        x: root.editing && !root.isPoint ? Math.max(4 - root.timelineX, Math.min(root.earWidth, root.timelineWidth - root.timelineX - width - 4)) : root.isPoint ? 4 : root.earWidth
        width: root.editing && !root.isPoint ? Math.min(Math.max(140, root.contentWidth), Math.max(40, root.timelineWidth - 8)) : parent.width - x - (root.isPoint ? 4 : root.earWidth)
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.isPoint || titleLoader.isEditState ? 2 : 4

        clip: true

        Loader {
            id: titleLoader

            property bool isEditState: false
            property real contentWidth: item ? (isEditState ? item.contentWidth + 1 : item.implicitWidth + 1) : 0
            property real contentHeight: item ? (isEditState ? item.contentHeight : item.implicitHeight) : 0

            x: Math.max(0, root.leftVisibleMargin - (root.isPoint ? root.x - root.pointAnchorX : root.x))
            width: parent.width
            height: parent.height

            sourceComponent: isEditState ? titleEditComp : titleComp

            function edit() {
                root.editStarted()

                titleLoader.isEditState = true
                titleLoader.item.currentText = root.title
                titleLoader.item.newTitle = root.title
                titleLoader.item.ensureActiveFocus()
            }

            Component {
                id: titleComp

                StyledTextLabel {
                    text: root.title

                    color: root.labelColor
                    font.pixelSize: root.isPoint ? ui.theme.bodyFont.pixelSize : 11

                    horizontalAlignment: Qt.AlignLeft
                    elide: Text.ElideNone
                    wrapMode: Text.NoWrap
                    maximumLineCount: 1
                }
            }

            Component {
                id: titleEditComp

                TextInputArea {
                    id: titleEdit

                    property string newTitle: ""
                    property real contentWidth: titleMetrics.implicitWidth
                    property real contentHeight: scrollableContentHeight
                    property bool _editEscaped: false

                    allowNewLineByEnter: false

                    Text {
                        id: titleMetrics
                        visible: false
                        wrapMode: Text.NoWrap
                        font: titleEdit.inputField.font
                        text: titleEdit.newTitle
                    }

                    background.color: root.backgroundColor
                    background.border.width: 0
                    background.radius: 0

                    inputField.color: root.labelColor
                    textSidePadding: 0

                    onTextChanged: function (text) {
                        titleEdit.newTitle = text
                    }

                    onAccepted: {
                        Qt.callLater(root.titleEditAccepted, newTitle)

                        titleLoader.isEditState = false
                    }

                    onEscaped: {
                        titleEdit._editEscaped = true
                    }

                    onFocusChanged: {
                        if (!titleEdit.focus) {
                            titleEdit.visible = false
                            Qt.callLater(function () {
                                var escaped = titleEdit._editEscaped
                                titleEdit._editEscaped = false
                                if (!escaped) {
                                    titleEdit.accepted()
                                } else {
                                    titleLoader.isEditState = false
                                }
                                root.editFinished()
                            })
                        }
                    }
                }
            }

            NavigationControl {
                id: titleEditNavCtrl
                name: "TitleEditNavCtrl"
                enabled: root.visible
                panel: root.navigationPanel
                column: 3

                accessible.enabled: titleEditNavCtrl.enabled

                onTriggered: {
                    titleLoader.edit()
                }
            }

            NavigationFocusBorder {
                navigationCtrl: titleEditNavCtrl

                anchors.topMargin: 1
                anchors.bottomMargin: 0
                radius: 5
            }
        }
    }
}
