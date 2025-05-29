import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Dialog {
    id: dialog
    title: isModifying ? "Modify Action" : "Create New Action"
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    property alias actionName: nameField.text
    property alias command: commandField.text
    property alias commandArgs: argumentsField.text
    property alias icon: iconField.text
    property bool isModifying: false

    // Custom button text
    Component.onCompleted: {
        let okButton = standardButton(Dialog.Ok)
        let cancelButton = standardButton(Dialog.Cancel)

        if (okButton) {
            okButton.text = Qt.binding(function() {
                return isModifying ? "Save" : "Create"
            })
            okButton.enabled = Qt.binding(function() {
                return nameField.text.length > 0 && commandField.text.length > 0
            })
        }

        if (cancelButton) {
            cancelButton.text = Qt.binding(function() {
                return isModifying ? "Delete" : "Cancel"
            })
        }
    }

    // Handle the cancel button differently when modifying
    onRejected: {
        if (isModifying) {
            deleteConfirmDialog.open()
        } else {
            close()
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
        ToolButton {
            icon.source: "qrc:/icons/browse.png"
            icon.width: 18
            icon.height: 18
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
        ToolButton {
            icon.source: "qrc:/icons/browse.png"
            icon.width: 18
            icon.height: 18
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

    Dialog {
        id: deleteConfirmDialog
        title: "Delete Action"
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Yes | Dialog.No

        Label {
            text: "Are you sure you want to delete this action?"
        }

        onAccepted: {
            dialog.deleteRequested()
            dialog.close()
        }
    }

    signal deleteRequested()

    function clearFields() {
        nameField.text = ""
        commandField.text = ""
        argumentsField.text = ""
        iconField.text = ""
        isModifying = false
    }
}
