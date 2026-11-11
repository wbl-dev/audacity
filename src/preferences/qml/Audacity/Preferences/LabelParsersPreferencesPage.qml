import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import Audacity.Preferences

PreferencesPage {
    id: root

    property var profiles: []
    property string currentId: ""
    property bool currentReadOnly: false
    property string feedback: ""

    LabelParsersPreferencesModel {
        id: parserModel
    }

    function refresh() {
        profiles = parserModel.profiles()
    }

    function formatSuffixes(suffixes) {
        return suffixes.length ? "." + suffixes.join(", .") : qsTrc("preferences", "None")
    }

    function editProfile(profile) {
        currentId = profile.id
        currentReadOnly = profile.readOnly
        nameField.text = profile.name
        suffixField.text = profile.suffixes
        enabledBox.checked = profile.enabled
        scriptArea.text = profile.script
        feedback = ""
    }

    function saveProfile(allowOverride) {
        const result = parserModel.save(currentId, nameField.text, suffixField.text, enabledBox.checked, scriptArea.text, allowOverride)
        if (result.requiresOverride) {
            overrideDialog.open()
            return
        }
        feedback = result.ok ? qsTrc("preferences", "Parser saved.") : result.error
        if (result.ok) {
            currentId = result.id
            refresh()
        }
    }

    Component.onCompleted: refresh()

    ColumnLayout {
        width: parent.width
        spacing: 10

        Label {
            text: qsTrc("preferences", "Label file parsers")
            font.bold: true
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: qsTrc("preferences", "Save a JavaScript file named after its suffix, such as ann.js. Scripts are scanned at startup or when you rescan. The built-in TXT parser can be duplicated or overridden by txt.js.")
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WrapAnywhere
            text: parserModel.scriptsDirectory()
        }
        RowLayout {
            Button {
                text: qsTrc("preferences", "Open scripts folder")
                onClicked: {
                    if (!parserModel.openScriptsDirectory())
                        root.feedback = qsTrc("preferences", "Could not open scripts folder.")
                }
            }
            Button {
                text: qsTrc("preferences", "Rescan scripts")
                onClicked: {
                    const result = parserModel.rescan()
                    root.refresh()
                    const summary = qsTrc("preferences", "Currently supported: %1\nAdded: %2\nRemoved: %3")
                                    .arg(root.formatSuffixes(result.supported))
                                    .arg(root.formatSuffixes(result.added))
                                    .arg(root.formatSuffixes(result.removed))
                    root.feedback = result.ok ? summary : result.error + "\n" + summary
                }
            }
        }

        ListView {
            id: profileList
            Layout.fillWidth: true
            Layout.preferredHeight: 115
            clip: true
            model: root.profiles
            delegate: ItemDelegate {
                required property var modelData
                width: profileList.width
                text: modelData.name + " (" + modelData.suffixes + ")" + (modelData.enabled ? "" : " — disabled")
                onClicked: root.editProfile(modelData)
            }
        }

        RowLayout {
            Button {
                text: qsTrc("preferences", "Add")
                onClicked: {
                    root.currentId = ""
                    root.currentReadOnly = false
                    nameField.text = ""
                    suffixField.text = ""
                    enabledBox.checked = true
                    scriptArea.text = "function parse(input) {\n    return { trackName: input.fileName, labels: [] };\n}"
                    root.feedback = ""
                }
            }
            Button {
                text: qsTrc("preferences", "Duplicate")
                enabled: root.currentId !== ""
                onClicked: {
                    root.currentId = ""
                    root.currentReadOnly = false
                    nameField.text += " copy"
                }
            }
            Button {
                text: qsTrc("preferences", "Remove")
                enabled: root.currentId !== "" && !root.currentReadOnly
                onClicked: {
                    const result = parserModel.remove(root.currentId)
                    root.feedback = result.ok ? qsTrc("preferences", "Parser removed.") : result.error
                    if (result.ok) {
                        root.currentId = ""
                        root.refresh()
                    }
                }
            }
        }

        Label {
            text: qsTrc("preferences", "Display name")
        }
        TextField {
            id: nameField
            Layout.fillWidth: true
            readOnly: root.currentReadOnly
        }
        Label {
            text: qsTrc("preferences", "Suffixes (comma separated, for example ann, srt)")
        }
        TextField {
            id: suffixField
            Layout.fillWidth: true
            readOnly: root.currentReadOnly
        }
        CheckBox {
            id: enabledBox
            text: qsTrc("preferences", "Enabled")
            enabled: !root.currentReadOnly
        }
        Label {
            text: qsTrc("preferences", "JavaScript")
        }
        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: 220
            TextArea {
                id: scriptArea
                wrapMode: TextEdit.NoWrap
                readOnly: root.currentReadOnly
                font.family: "monospace"
            }
        }

        RowLayout {
            Button {
                text: qsTrc("preferences", "Test with sample file…")
                onClicked: sampleDialog.open()
            }
            Button {
                text: qsTrc("preferences", "Save parser")
                enabled: !root.currentReadOnly
                onClicked: root.saveProfile(false)
            }
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: root.feedback
        }
    }

    FileDialog {
        id: sampleDialog
        title: qsTrc("preferences", "Choose a sample label file")
        fileMode: FileDialog.OpenFile
        onAccepted: {
            const result = parserModel.test(scriptArea.text, selectedFile.toString())
            root.feedback = result.ok ? qsTrc("preferences", "%1 labels parsed for %2. First rows: %3").arg(result.count).arg(result.trackName).arg(JSON.stringify(result.labels)) : result.error
        }
    }

    Dialog {
        id: overrideDialog
        modal: true
        title: qsTrc("preferences", "Override built-in TXT parser?")
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: root.saveProfile(true)
        Label {
            text: qsTrc("preferences", "This parser will replace the bundled TXT importer while enabled.")
            wrapMode: Text.WordWrap
            width: 320
        }
    }
}
