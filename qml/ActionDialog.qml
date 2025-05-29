import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: dialog
    title: "Create New Action"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    property alias actionName: nameField.text
    property alias command: commandField.text
    property alias commandArgs: argumentsField.text
    property alias icon: iconField.text

    // Custom button text
    Component.onCompleted: {
        let okButton = standardButton(Dialog.Ok)
        if (okButton) {
            okButton.text = "Create"
            okButton.enabled = Qt.binding(function() {
                return nameField.text.length > 0 && commandField.text.length > 0
            })
        }
    }

    width: 500
    height: 350

    GridLayout {
        anchors.fill: parent
        columns: 3
        columnSpacing: 10
        rowSpacing: 10

        Label { text: "Name:" }
        TextField {
            id: nameField
            Layout.fillWidth: true
            Layout.columnSpan: 2
            placeholderText: "Action name"
        }

        Label { text: "Command:" }
        TextField {
            id: commandField
            Layout.fillWidth: true
            placeholderText: "Command to execute"
        }
        Button {
            text: "Browse..."
            onClicked: commandFileDialog.open()
        }

        Label { text: "Arguments:" }
        TextField {
            id: argumentsField
            Layout.fillWidth: true
            Layout.columnSpan: 2
            placeholderText: "Command arguments (optional)"
        }

        Label { text: "Icon:" }
        TextField {
            id: iconField
            Layout.fillWidth: true
            placeholderText: "Icon path (optional)"
        }
        Button {
            text: "Browse..."
            onClicked: iconFileDialog.open()
        }
    }

    FileDialog {
        id: commandFileDialog
        title: "Select Command/Executable"
        nameFilters: ["Executable files (*.exe *.bat *.cmd)", "All files (*)"]
        onAccepted: {
            commandField.text = selectedFile.toString().replace("file:///", "")
        }
    }

    FileDialog {
        id: iconFileDialog
        title: "Select Icon"
        nameFilters: ["Image files (*.png *.jpg *.jpeg *.ico *.svg)", "All files (*)"]
        onAccepted: {
            iconField.text = selectedFile.toString().replace("file:///", "")
        }
    }

    function clearFields() {
        nameField.text = ""
        commandField.text = ""
        argumentsField.text = ""
        iconField.text = ""
    }
}
