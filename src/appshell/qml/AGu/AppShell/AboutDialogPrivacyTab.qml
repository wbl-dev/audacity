/*
* Audacity: A Digital Audio Editor
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15

import Muse.Ui 1.0
import Muse.UiComponents

import AGu.AppShell

ColumnLayout {
    id: root

    required property AboutModel model

    spacing: prv.contentSpacing

    QtObject {
        id: prv

        readonly property int versionTextSpacing: 12
        readonly property int contentSpacing: 16
        readonly property int contentMargin: 16
        readonly property int contentTextMargin: 12

        readonly property string privacyTitle: qsTrc("appshell/about", "Privacy")
        readonly property string workbenchPrivacySubtitle: qsTrc("appshell/about", "Optional crash reporting requires network access. Projects and audio editing stay on this device.")
    }

    Column {
        Layout.fillWidth: true
        Layout.preferredHeight: implicitHeight

        spacing: prv.versionTextSpacing

        StyledTextLabel {
            width: parent.width

            horizontalAlignment: Text.AlignHCenter

            text: prv.privacyTitle
            font: ui.theme.tabBoldFont
        }

        StyledTextLabel {
            width: parent.width

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText

            text: prv.workbenchPrivacySubtitle
            font: ui.theme.bodyFont
        }
    }

    StyledFlickable {
        id: legalFlickable

        Layout.fillWidth: true
        Layout.fillHeight: true

        contentHeight: gplContainer.height

        Rectangle {
            id: gplContainer

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: prv.contentMargin
            anchors.rightMargin: prv.contentMargin

            height: gplInner.height + prv.contentTextMargin * 2

            color: ui.theme.backgroundSecondaryColor

            Column {
                id: gplInner

                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                anchors.margins: prv.contentTextMargin

                StyledTextLabel {
                    width: parent.width

                    horizontalAlignment: Text.AlignLeft
                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap

                    text: root.model.gplText()

                    font: ui.theme.bodyFont
                }
            }
        }
    }
}
