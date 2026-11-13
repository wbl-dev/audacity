import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import Audacity.ProjectScene

Item {
    id: root

    property string title: ""
    property bool isSelected: false
    property bool isFocused: false
    property bool enableCursorInteraction: !selectionInProgress && !selectionEditInProgress && !verticalSelectionEditInProgress
    property var labelKey: null

    required property bool selectionInProgress
    required property bool selectionEditInProgress
    required property bool verticalSelectionEditInProgress

    property real leftVisibleMargin: 0

    property bool isLeftLinked: false
    property bool isRightLinked: false
    property bool isLinkedActive: false
    property bool isPoint: false
    property real pixelsPerSecond: 100

    readonly property bool compactPoint: isPoint && pixelsPerSecond < 40
    readonly property bool compactRange: !isPoint && width < header.contentWidth + 6
    readonly property real pointPositionX: root.parent ? root.parent.x + root.parent.width / 2 : -1
    readonly property bool pointInViewport: root.container && pointPositionX >= 0 && pointPositionX <= root.container.width
    readonly property bool rangeInViewport: root.container && root.parent && root.parent.x + root.width > 0 && root.parent.x < root.container.width

    property int visualWidth: prv.isPoint ? (compactPoint ? 24 : 16 + header.width) : Math.max(width, 8)
    readonly property int headerDefaultHeight: 22
    readonly property int pointCardHeight: 18

    property var container: null

    property bool hover: root.containsMouse || root.headerHovered
    property bool headerHovered: false
    property bool containsMouse: false

    property color labelColor: null
    property real startTime: 0
    property real endTime: 0

    component TimeSelection: QtObject {
        property bool isDataSelected: false
        property real startTime: 0
        property real endTime: 0
        property real start: 0
        property real width: 0
        property color color: "transparent"
    }

    readonly property TimeSelection timeSelection: TimeSelection {}

    property alias navigation: navCtrl

    height: prv.isPoint ? pointCardHeight : headerDefaultHeight

    signal requestSelected
    signal requestSingleSelected
    signal requestSelectionReset

    signal requestSelectLinkedGroup(bool rightSide)

    signal titleEditStarted
    signal titleEditAccepted(var newTitle)
    signal titleEditCanceled
    signal titleEditFinished

    signal labelItemMousePositionChanged(real x, real y)

    signal labelStartEditRequested
    signal labelEndEditRequested
    signal labelCancelDragEditRequested

    signal labelLeftStretchRequested(bool unlink, bool completed)
    signal labelRightStretchRequested(bool unlink, bool completed)

    signal activateLeftLinkedLabel
    signal activateRightLinkedLabel
    signal deactivateLinkedLabel

    QtObject {
        id: prv

        property bool isPoint: root.isPoint
        readonly property int earWidth: 7

        readonly property bool hasTimeSelection: root.timeSelection.isDataSelected && root.timeSelection.endTime > root.timeSelection.startTime
        readonly property bool leftEdgeInSelection: hasTimeSelection && (isPoint ? root.timeSelection.startTime < root.startTime : root.timeSelection.startTime <= root.startTime) && root.startTime < root.timeSelection.endTime
        readonly property bool rightEdgeInSelection: isPoint ? leftEdgeInSelection : hasTimeSelection && root.timeSelection.startTime < root.endTime && root.endTime <= root.timeSelection.endTime
        readonly property bool overlapsSelection: isPoint ? leftEdgeInSelection : hasTimeSelection && root.timeSelection.startTime < root.endTime && root.startTime < root.timeSelection.endTime

        property color backgroundColor: root.labelColor
        property color leftEarBackgroundColor: root.labelColor
        property color rightEarBackgroundColor: root.labelColor
    }

    function setContainsMouse(containsMouse) {
        if (!root.enableCursorInteraction) {
            return
        }

        root.containsMouse = containsMouse
    }

    function editTitle() {
        header.edit()
    }

    function openContextMenu() {
        openContextMenuAt(width - 4, 4)
    }

    function openContextMenuAt(x, y) {
        root.requestSelected()
        labelContextMenuLoader.show(Qt.point(x, y), labelContextMenuModel.items)
    }

    // Navigation support
    NavigationControl {
        id: navCtrl
        name: root.objectName || "LabelItem"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Button
        accessible.name: qsTrc("projectscene", "Label: %1").arg(root.title)

        onActiveChanged: function (active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: {
            root.requestSingleSelected()
        }
    }

    NavigationFocusBorder {
        anchors.leftMargin: -leftEar.width - border.width
        anchors.rightMargin: -rightEar.width - border.width

        navigationCtrl: prv.isPoint ? null : navCtrl

        border.color: ui.theme.fontPrimaryColor
        border.width: 2
        radius: 4
    }

    NavigationFocusBorder {
        parent: header

        navigationCtrl: prv.isPoint ? navCtrl : null

        border.color: ui.theme.fontPrimaryColor
        border.width: 2
        radius: 4
    }

    // panel for navigating within the label's items
    property NavigationPanel labelNavigationPanel: NavigationPanel {
        name: "LabelNavigationPanel"
        enabled: navCtrl.active
        direction: NavigationPanel.Horizontal
        section: navigation.panel ? navigation.panel.section : null
        onActiveChanged: function (active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onNavigationEvent: function (event) {
            if (event.type === NavigationEvent.Escape) {
                navCtrl.requestActive()
            }
        }
    }

    LabelContextMenuModel {
        id: labelContextMenuModel
        labelKey: root.labelKey

        onLabelEditRequested: {
            root.editTitle()
        }
    }

    ContextMenuLoader {
        id: labelContextMenuLoader

        onHandleMenuItem: function (itemId) {
            labelContextMenuModel.handleMenuItem(itemId)
        }
    }

    Component.onCompleted: {
        labelContextMenuModel.load()
    }

    Component.onDestruction: {
        //! NOTE The outer component uses this information to handle the current cursor.
        // It is important to cleanup this state before removing the component
        // to prevent the cursor from being stuck in the wrong state.
        root.headerHovered = false
    }

    // Left Ear
    LabelEar {
        id: leftEar

        //! To draw on top of other labels, we need to change the parent and increase z
        parent: root.container
        x: root.parent.x - prv.earWidth - (prv.isPoint ? 0 : 1)
        y: root.parent.y
        z: root.parent.z

        height: root.headerDefaultHeight
        enabled: root.enabled
        visible: !prv.isPoint
        opacity: root.isSelected || root.headerHovered || hovered ? 1 : 0

        isRight: false
        enableCursorInteraction: root.enableCursorInteraction
        backgroundColor: prv.leftEdgeInSelection ? root.timeSelection.color : prv.leftEarBackgroundColor
        isSelected: root.isSelected
        isLinked: root.isLeftLinked

        onHoveredChanged: {
            if (hovered && isLinked) {
                root.activateLeftLinkedLabel()
            } else if (!hovered && isLinked) {
                root.deactivateLinkedLabel()
            }
        }

        onStretchStartRequested: {
            root.requestSingleSelected()
            root.labelStartEditRequested()
        }

        onStretchMousePositionChanged: function (x, y) {
            root.labelItemMousePositionChanged(x - root.parent.x, y)
        }

        onStretchRequested: function (completed) {
            root.labelLeftStretchRequested(true /*unlink*/, completed)
        }

        onStretchEndRequested: {
            root.labelEndEditRequested()
        }

        onStretchCanceled: {
            root.labelCancelDragEditRequested()
        }
    }

    // Right Ear
    LabelEar {
        id: rightEar

        //! To draw on top of other labels, we need to change the parent and increase z
        parent: root.container
        x: root.parent.x + root.width + (prv.isPoint ? 0 : 1)
        y: root.parent.y
        z: root.parent.z

        height: root.headerDefaultHeight
        enabled: root.enabled
        visible: !prv.isPoint
        opacity: root.isSelected || root.headerHovered || hovered ? 1 : 0

        isRight: true
        enableCursorInteraction: root.enableCursorInteraction
        backgroundColor: prv.rightEdgeInSelection ? root.timeSelection.color : prv.rightEarBackgroundColor
        isSelected: root.isSelected
        isLinked: root.isRightLinked

        onHoveredChanged: {
            if (hovered && isLinked) {
                root.activateRightLinkedLabel()
            } else if (!hovered && isLinked) {
                root.deactivateLinkedLabel()
            }
        }

        onStretchStartRequested: {
            root.requestSingleSelected()
            root.labelStartEditRequested()
        }

        onStretchMousePositionChanged: function (x, y) {
            root.labelItemMousePositionChanged(x - root.parent.x, y)
        }

        onStretchRequested: function (completed) {
            root.labelRightStretchRequested(true /*unlink*/, completed)
        }

        onStretchEndRequested: {
            root.labelEndEditRequested()
        }

        onStretchCanceled: {
            root.labelCancelDragEditRequested()
        }
    }

    // Main Label Header
    LabelHeader {
        id: header

        parent: prv.isPoint && root.container ? root.container : root
        z: prv.isPoint && (root.isSelected || root.headerHovered || header.hovered || pointMarker.hovered) ? 1000 : 0
        enabled: root.enabled

        width: prv.isPoint ? Math.min(contentWidth, Math.max(40, parent.width - 8)) : Math.max(root.width, 8)
        height: prv.isPoint ? root.pointCardHeight : root.headerDefaultHeight

        title: root.title

        isPoint: prv.isPoint
        isSelected: root.isSelected
        compactRange: root.compactRange
        showTitle: prv.isPoint || !root.compactRange
        pointAnchorX: root.pointPositionX
        pointAnchorY: root.parent ? root.parent.y + 3 : 0
        timelineX: root.parent ? root.parent.x : 0
        timelineWidth: root.container ? root.container.width : 0

        backgroundColor: prv.isPoint ? ui.theme.backgroundSecondaryColor : prv.backgroundColor
        labelColor: prv.isPoint ? ui.theme.fontPrimaryColor : ui.theme.extra["black_color"]

        timeSelection.start: prv.isPoint ? 0 : root.timeSelection.start
        timeSelection.width: !prv.overlapsSelection ? 0 : prv.isPoint ? header.width : root.timeSelection.width
        timeSelection.color: root.timeSelection.color

        earWidth: prv.earWidth

        leftVisibleMargin: root.leftVisibleMargin

        enableCursorInteraction: root.enableCursorInteraction

        navigationPanel: root.labelNavigationPanel

        visible: root.visible && (!prv.isPoint || root.pointInViewport) && (!prv.isPoint || !root.compactPoint || root.isSelected || root.headerHovered || header.hovered || pointMarker.hovered || editing)

        onTitleEditAccepted: function (newTitle) {
            root.titleEditAccepted(newTitle)
        }

        onEditStarted: {
            root.titleEditStarted()
        }

        onEditFinished: {
            root.titleEditFinished()
        }

        onRequestSingleSelected: {
            root.requestSingleSelected()
        }

        onMousePositionChanged: function (x, y) {
            const pos = header.mapToItem(root.container, x, y)
            root.labelItemMousePositionChanged(pos.x - root.parent.x, pos.y)
        }

        onHeaderHoveredChanged: function (value) {
            root.headerHovered = value
        }
    }

    Rectangle {
        id: compactRangeTitle

        parent: root.container
        z: 2000
        width: root.container ? Math.min(Math.max(0, root.container.width - 8), Math.max(64, fullTitle.implicitWidth + 12)) : 0
        height: 24
        x: root.container ? Math.max(4, Math.min(root.parent.x, root.container.width - width - 4)) : 0
        y: root.container && root.parent.y + root.headerDefaultHeight + height + 2 <= root.container.height ? root.parent.y + root.headerDefaultHeight + 2 : Math.max(0, root.parent.y - height - 2)
        radius: 4
        color: ui.theme.backgroundSecondaryColor
        border.color: ui.theme.strokeColor
        visible: root.visible && root.rangeInViewport && root.title.length > 0 && root.compactRange && (root.isSelected || root.headerHovered) && !header.editing

        StyledTextLabel {
            id: fullTitle
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 6
            text: root.title
            color: ui.theme.fontPrimaryColor
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
            maximumLineCount: 1
        }
    }

    // Point's Stalk
    Item {
        id: pointMarker
        readonly property bool hovered: pointMarkerMouseArea.containsMouse
        parent: root.container
        width: 24
        height: 24
        x: root.pointPositionX - width / 2
        y: root.parent.y
        z: root.parent.z + 1
        enabled: root.enabled
        visible: prv.isPoint

        Rectangle {
            anchors.centerIn: parent
            width: 12
            height: 12
            radius: width / 2
            color: prv.leftEarBackgroundColor
            border.color: root.isSelected ? ui.theme.fontPrimaryColor : root.labelColor
            border.width: root.isSelected ? 2 : 1
        }

        MouseArea {
            id: pointMarkerMouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.OpenHandCursor
            onPressed: function (e) {
                const titleCardHandledPress = header.hovered
                root.headerHovered = true
                const pos = pointMarker.mapToItem(root.container, e.x, e.y)
                root.labelItemMousePositionChanged(pos.x - root.parent.x, pos.y)
                // The title card can overlap the marker near the viewport edge.
                // Its MouseArea has already selected this point in that case.
                if (!titleCardHandledPress) {
                    root.requestSingleSelected()
                }
                e.accepted = false
            }
            onPositionChanged: function (e) {
                const pos = pointMarker.mapToItem(root.container, e.x, e.y)
                root.labelItemMousePositionChanged(pos.x - root.parent.x, pos.y)
                e.accepted = false
            }
            onContainsMouseChanged: root.headerHovered = containsMouse
        }
    }

    LabelStalk {
        id: pointStalk

        //! To draw on top of other labels, we need to change the parent and increase z
        parent: root.container
        width: 2
        x: root.pointPositionX - width / 2
        y: root.parent.y
        z: root.parent.z

        height: 24

        enabled: root.enabled
        isForPoint: true
        enableCursorInteraction: false
        backgroundColor: prv.leftEdgeInSelection ? root.timeSelection.color : leftEar.hovered ? prv.leftEarBackgroundColor : prv.rightEarBackgroundColor

        visible: prv.isPoint

        onHeaderHoveredChanged: function (value) {
            root.headerHovered = value
        }

        onRequestSelected: {
            root.requestSingleSelected()
        }

        onMousePositionChanged: function (x, y) {
            root.labelItemMousePositionChanged(x - root.parent.x, y)
        }
    }

    // Left Stalk
    LabelStalk {
        id: leftStalk

        property bool isLinked: root.isLeftLinked

        //! To draw on top of other labels, we need to change the parent and increase z
        parent: root.container
        x: root.parent.x - 1
        y: root.parent.y
        z: root.parent.z

        height: root.headerDefaultHeight

        enabled: root.enabled
        isRight: false
        enableCursorInteraction: root.enableCursorInteraction
        backgroundColor: prv.leftEdgeInSelection ? root.timeSelection.color : prv.leftEarBackgroundColor
        isSelected: root.isSelected

        visible: !prv.isPoint
        opacity: root.isSelected || root.headerHovered || hovered ? 1 : 0

        onHeaderHoveredChanged: function (value) {
            root.headerHovered = value
        }

        onHoveredChanged: {
            if (hovered && isLinked) {
                root.activateLeftLinkedLabel()
            } else if (!hovered && isLinked) {
                root.deactivateLinkedLabel()
            }
        }

        onRequestSelected: {
            root.requestSelectLinkedGroup(false /*rightSide*/)
        }

        onMousePositionChanged: function (x, y) {
            root.labelItemMousePositionChanged(x - root.parent.x, y)
        }
    }

    // Right Stalk
    LabelStalk {
        id: rightStalk

        property bool isLinked: root.isRightLinked

        //! To draw on top of other labels, we need to change the parent and increase z
        parent: root.container
        x: root.parent.x + root.width
        y: root.parent.y
        z: root.parent.z

        height: root.headerDefaultHeight

        enabled: root.enabled
        isRight: true
        enableCursorInteraction: root.enableCursorInteraction
        backgroundColor: prv.rightEdgeInSelection ? root.timeSelection.color : prv.rightEarBackgroundColor
        isSelected: root.isSelected

        visible: !prv.isPoint
        opacity: root.isSelected || root.headerHovered || hovered ? 1 : 0

        onHeaderHoveredChanged: function (value) {
            root.headerHovered = value
        }

        onHoveredChanged: {
            if (hovered && isLinked) {
                root.activateRightLinkedLabel()
            } else if (!hovered && isLinked) {
                root.deactivateLinkedLabel()
            }
        }

        onRequestSelected: {
            root.requestSelectLinkedGroup(true /*rightSide*/)
        }

        onMousePositionChanged: function (x, y) {
            root.labelItemMousePositionChanged(x - root.parent.x, y)
        }
    }

    states: [
        State {
            name: "NORMAL"
            when: !root.isSelected && !root.headerHovered && !leftEar.hovered && !leftStalk.hovered && !rightEar.hovered && !rightStalk.hovered && ((root.isLeftLinked || root.isRightLinked) && !root.isLinkedActive)
            PropertyChanges {
                target: prv
                backgroundColor: root.labelColor
                leftEarBackgroundColor: prv.backgroundColor
                rightEarBackgroundColor: prv.backgroundColor
            }
        },
        State {
            name: "SELECTED"
            when: root.isSelected && !root.headerHovered
            PropertyChanges {
                target: prv
                backgroundColor: ui.blendColors(ui.theme.extra["white_color"], root.labelColor, 0.4)
                leftEarBackgroundColor: prv.backgroundColor
                rightEarBackgroundColor: prv.backgroundColor
            }
        },
        State {
            name: "NORMAL_HOVERED"
            when: !root.isSelected && root.headerHovered
            PropertyChanges {
                target: prv
                backgroundColor: ui.blendColors(ui.theme.extra["white_color"], root.labelColor, 0.7)
                leftEarBackgroundColor: prv.backgroundColor
                rightEarBackgroundColor: prv.backgroundColor
            }
        },
        State {
            name: "LEFT_EAR_HOVERED"
            when: (!root.isSelected && (leftEar.hovered || leftStalk.hovered) && !root.headerHovered) || (root.isLeftLinked && root.isLinkedActive)
            PropertyChanges {
                target: prv
                backgroundColor: ui.blendColors(ui.theme.extra["white_color"], root.labelColor, 0.7)
                leftEarBackgroundColor: ui.blendColors(ui.theme.extra["white_color"], root.labelColor, 0.3)
                rightEarBackgroundColor: prv.backgroundColor
            }
        },
        State {
            name: "RIGHT_EAR_HOVERED"
            when: !root.isSelected && (rightEar.hovered || rightStalk.hovered || (root.isRightLinked && root.isLinkedActive)) && !root.headerHovered
            PropertyChanges {
                target: prv
                backgroundColor: ui.blendColors(ui.theme.extra["white_color"], root.labelColor, 0.7)
                leftEarBackgroundColor: prv.backgroundColor
                rightEarBackgroundColor: ui.blendColors(ui.theme.extra["white_color"], root.labelColor, 0.3)
            }
        },
        State {
            name: "SELECTED_HOVERED"
            when: root.isSelected && root.headerHovered
            PropertyChanges {
                target: prv
                backgroundColor: ui.blendColors(ui.theme.extra["white_color"], root.labelColor, 0.2)
                leftEarBackgroundColor: prv.backgroundColor
                rightEarBackgroundColor: prv.backgroundColor
            }
        }
    ]
}
