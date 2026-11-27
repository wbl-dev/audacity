/*
* Audacity: A Digital Audio Editor
*/
import QtQuick

import Muse.UiComponents

import AGu.ProjectScene

TrackItem {
    id: root

    signal addLabelToSelectionRequested
    signal importLabelsRequested

    extraControlsComponent: Component {
        Row {
            width: parent.width
            spacing: 4
            opacity: root.collapsed ? 0 : 1
            visible: opacity !== 0
            Behavior on opacity {
                OpacityAnimator {
                    duration: 100
                }
            }

            FlatButton {
                width: (parent.width - parent.spacing) / 2
                height: 24
                isNarrow: true
                text: qsTrc("projectscene", "Add label")
                navigation.panel: root.headerNavigationPanel
                navigation.order: root.extraControlsNavigationStart
                navigation.enabled: !root.collapsed
                onClicked: root.addLabelToSelectionRequested()
            }

            FlatButton {
                width: (parent.width - parent.spacing) / 2
                height: 24
                isNarrow: true
                text: qsTrc("projectscene", "Import…")
                navigation.panel: root.headerNavigationPanel
                navigation.order: root.extraControlsNavigationStart + 1
                navigation.enabled: !root.collapsed
                onClicked: root.importLabelsRequested()
            }
        }
    }
}
