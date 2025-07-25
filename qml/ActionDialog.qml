import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Dialogs

Popup {
    id: popup
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    property alias actionName: nameField.text
    property alias command: commandField.text
    property alias commandArgs: argumentsField.text
    property alias icon: iconField.text
    property alias actionType: typeComboBox.currentIndex
    property alias mediaKey: mediaKeyComboBox.currentIndex
    property alias shortcutKey: shortcutField.text
    property bool isModifying: false
    property int labelWidth: 100
    Material.background: UserSettings.darkMode ? "#1C1C1C" : "#E3E3E3"

    width: 500
    height: mainContent.implicitHeight + 80

    anchors.centerIn: Overlay.overlay

    ColumnLayout {
        id: mainContent
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        // Header
        Label {
            text: popup.isModifying ? "Modify Action" : "Create New Action"
            font.pixelSize: 18
            font.bold: true
            Layout.fillWidth: true
        }

        // Common fields
        Pane {
            Layout.fillWidth: true
            Material.background: UserSettings.darkMode ? "#2B2B2B" : "#FFFFFF"
            Material.elevation: 6
            Material.roundedScale: Material.ExtraSmallScale

            GridLayout {
                anchors.fill: parent
                columns: 3
                columnSpacing: 10
                rowSpacing: 10

                Label {
                    text: "Name:"
                    Layout.preferredWidth: popup.labelWidth
                }
                TextField {
                    Layout.preferredHeight: 35
                    id: nameField
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    placeholderText: "Action name"
                }

                Label {
                    text: "Type:"
                    Layout.preferredWidth: popup.labelWidth
                }
                ComboBox {
                    Layout.preferredHeight: 35
                    id: typeComboBox
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    model: ["Command", "Media Key", "Shortcut"]
                    currentIndex: 0
                }

                Label {
                    text: "Icon:"
                    Layout.preferredWidth: popup.labelWidth
                }
                TextField {
                    Layout.preferredHeight: 35
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
        }

        MenuSeparator { Layout.fillWidth: true }

        Pane {
            Layout.fillWidth: true
            Material.background: UserSettings.darkMode ? "#2B2B2B" : "#FFFFFF"
            Material.elevation: 6
            Material.roundedScale: Material.ExtraSmallScale
            visible: typeComboBox.currentIndex === 0

            GridLayout {
                anchors.fill: parent
                id: commandLayout
                columns: 3
                columnSpacing: 10
                rowSpacing: 10

                Label {
                    text: "Command:"
                    Layout.preferredWidth: popup.labelWidth
                }
                TextField {
                    Layout.preferredHeight: 35
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

                Label {
                    text: "Arguments:"
                    Layout.preferredWidth: popup.labelWidth
                }
                TextField {
                    Layout.preferredHeight: 35
                    id: argumentsField
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    placeholderText: "Command arguments (optional)"
                }
            }
        }

        Pane {
            Layout.fillWidth: true
            Material.background: UserSettings.darkMode ? "#2B2B2B" : "#FFFFFF"
            Material.elevation: 6
            Material.roundedScale: Material.ExtraSmallScale
            visible: typeComboBox.currentIndex === 1

            // Media Key layout
            GridLayout {
                id: mediaLayout
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 10

                Label {
                    text: "Media Key:"
                    Layout.preferredWidth: popup.labelWidth
                }
                ComboBox {
                    Layout.preferredHeight: 35
                    id: mediaKeyComboBox
                    Layout.fillWidth: true
                    model: [
                        "Play/Pause",
                        "Stop",
                        "Next Track",
                        "Previous Track",
                        "Volume Up",
                        "Volume Down",
                        "Volume Mute"
                    ]
                    currentIndex: 0
                }
            }
        }

        Pane {
            Layout.fillWidth: true
            Material.background: UserSettings.darkMode ? "#2B2B2B" : "#FFFFFF"
            Material.elevation: 6
            Material.roundedScale: Material.ExtraSmallScale
            visible: typeComboBox.currentIndex === 2

            // Shortcut layout
            GridLayout {
                id: shortcutLayout
                anchors.fill: parent
                columns: 2
                columnSpacing: 10
                rowSpacing: 10

                Label {
                    text: "Shortcut:"
                    Layout.preferredWidth: popup.labelWidth
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    ComboBox {
                        Layout.preferredHeight: 35
                        id: modifier1ComboBox
                        Layout.preferredWidth: 80
                        model: ["", "Ctrl", "Alt", "Shift", "Meta"]
                        currentIndex: 0
                        onCurrentTextChanged: shortcutLayout.updateShortcutText()
                    }

                    Label {
                        text: "+"
                        visible: modifier1ComboBox.currentIndex > 0
                        opacity: 0.6
                    }

                    ComboBox {
                        Layout.preferredHeight: 35
                        id: modifier2ComboBox
                        Layout.preferredWidth: 80
                        model: ["", "Ctrl", "Alt", "Shift", "Meta"]
                        currentIndex: 0
                        onCurrentTextChanged: shortcutLayout.updateShortcutText()
                    }

                    Label {
                        text: "+"
                        visible: modifier2ComboBox.currentIndex > 0
                        opacity: 0.6
                    }

                    TextField {
                        Layout.preferredHeight: 35
                        id: keyTextField
                        Layout.fillWidth: true
                        placeholderText: "Click here and press a key, or type manually"

                        property bool captureMode: false

                        onActiveFocusChanged: {
                            if (activeFocus) {
                                captureMode = true
                                placeholderText = "Press a key or type manually..."
                            } else {
                                captureMode = false
                                placeholderText = "Click here and press a key, or type manually"
                            }
                        }

                        onTextChanged: shortcutLayout.updateShortcutText()

                        Keys.onPressed: function(event) {
                            if (!captureMode) return

                            // Skip modifier-only presses
                            if (event.key === Qt.Key_Control || event.key === Qt.Key_Alt ||
                                    event.key === Qt.Key_Shift || event.key === Qt.Key_Meta) {
                                event.accepted = true
                                return
                            }

                            // Get the key name
                            let keyName = getKeyName(event.key, event.text)
                            if (keyName) {
                                text = keyName
                                focus = false // Exit capture mode
                            }

                            event.accepted = true
                        }

                        function getKeyName(key, keyText) {
                            // Special keys mapping
                            const specialKeys = {
                                [Qt.Key_Tab]: "Tab",
                                [Qt.Key_Space]: "Space",
                                [Qt.Key_Return]: "Return",
                                [Qt.Key_Enter]: "Return",
                                [Qt.Key_Escape]: "Escape",
                                [Qt.Key_Backspace]: "Backspace",
                                [Qt.Key_Delete]: "Delete",
                                [Qt.Key_Insert]: "Insert",
                                [Qt.Key_Home]: "Home",
                                [Qt.Key_End]: "End",
                                [Qt.Key_PageUp]: "Page Up",
                                [Qt.Key_PageDown]: "Page Down",
                                [Qt.Key_Up]: "Up",
                                [Qt.Key_Down]: "Down",
                                [Qt.Key_Left]: "Left",
                                [Qt.Key_Right]: "Right",
                                [Qt.Key_F1]: "F1", [Qt.Key_F2]: "F2", [Qt.Key_F3]: "F3",
                                [Qt.Key_F4]: "F4", [Qt.Key_F5]: "F5", [Qt.Key_F6]: "F6",
                                [Qt.Key_F7]: "F7", [Qt.Key_F8]: "F8", [Qt.Key_F9]: "F9",
                                [Qt.Key_F10]: "F10", [Qt.Key_F11]: "F11", [Qt.Key_F12]: "F12",
                                [Qt.Key_Print]: "Print Screen",
                                [Qt.Key_Pause]: "Pause",
                                [Qt.Key_CapsLock]: "Caps Lock",
                                [Qt.Key_NumLock]: "Num Lock",
                                [Qt.Key_ScrollLock]: "Scroll Lock"
                            }

                            // Check if it's a special key
                            if (specialKeys[key]) {
                                return specialKeys[key]
                            }

                            // Handle regular alphanumeric keys
                            if (key >= Qt.Key_A && key <= Qt.Key_Z) {
                                return String.fromCharCode(key)
                            }

                            if (key >= Qt.Key_0 && key <= Qt.Key_9) {
                                return String.fromCharCode(key)
                            }

                            // For other printable characters, use the text
                            if (keyText && keyText.length === 1 && keyText.charCodeAt(0) >= 32) {
                                return keyText.toUpperCase()
                            }

                            return ""
                        }
                    }
                }

                Label {
                    text: "Preview:"
                    Layout.preferredWidth: popup.labelWidth
                }
                Label {
                    id: shortcutPreview
                    Layout.fillWidth: true
                    text: "No shortcut selected"
                    font.family: "monospace"
                    color: Material.accent
                    Layout.preferredHeight: 30
                    verticalAlignment: Text.AlignVCenter
                    background: Rectangle {
                        color: Material.backgroundColor
                        border.color: Material.frameColor
                        border.width: 1
                        radius: 4
                    }
                    leftPadding: 8
                }

                Label {
                    Layout.columnSpan: 2
                    text: "Examples: Tab, A, F5, Space, Return, Escape, Delete, Home, End, Up, Down, Left, Right"
                    font.pixelSize: 10
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                function updateShortcutText() {
                    let parts = []

                    if (modifier1ComboBox.currentText !== "") {
                        parts.push(modifier1ComboBox.currentText)
                    }

                    if (modifier2ComboBox.currentText !== "" &&
                            modifier2ComboBox.currentText !== modifier1ComboBox.currentText) {
                        parts.push(modifier2ComboBox.currentText)
                    }

                    if (keyTextField.text.trim() !== "") {
                        parts.push(keyTextField.text.trim())
                    }

                    if (parts.length > 0) {
                        shortcutField.text = parts.join("+")
                        shortcutPreview.text = parts.join(" + ")
                    } else {
                        shortcutField.text = ""
                        shortcutPreview.text = "No shortcut selected"
                    }
                }

                function parseShortcut(shortcutString) {
                    if (!shortcutString) return

                    let parts = shortcutString.split("+")
                    let modifiers = []
                    let key = ""

                    for (let part of parts) {
                        part = part.trim()
                        if (["Ctrl", "Alt", "Shift", "Meta"].includes(part)) {
                            modifiers.push(part)
                        } else {
                            key = part
                        }
                    }

                    // Set modifiers
                    modifier1ComboBox.currentIndex = 0
                    modifier2ComboBox.currentIndex = 0

                    if (modifiers.length > 0) {
                        let index1 = modifier1ComboBox.model.indexOf(modifiers[0])
                        if (index1 >= 0) modifier1ComboBox.currentIndex = index1
                    }

                    if (modifiers.length > 1) {
                        let index2 = modifier2ComboBox.model.indexOf(modifiers[1])
                        if (index2 >= 0) modifier2ComboBox.currentIndex = index2
                    }

                    // Set key
                    keyTextField.text = key || ""
                }
            }
        }

        // Buttons
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Item { Layout.fillWidth: true } // Spacer

            Button {
                text: popup.isModifying ? "Delete" : "Cancel"
                onClicked: {
                    if (popup.isModifying) {
                        deleteConfirmDialog.open()
                    } else {
                        popup.close()
                    }
                }
            }

            Button {
                text: popup.isModifying ? "Save" : "Create"
                enabled: nameField.text.length > 0 && popup.isValidConfiguration()
                highlighted: true
                onClicked: {
                    if (popup.isModifying) {
                        popup.saveAction()
                    } else {
                        popup.createAction()
                    }
                    popup.close()
                }
            }
        }
    }

    function isValidConfiguration() {
        if (typeComboBox.currentIndex === 0) { // Command
            return commandField.text.length > 0
        } else if (typeComboBox.currentIndex === 1) { // Media Key
            return mediaKeyComboBox.currentIndex >= 0
        } else if (typeComboBox.currentIndex === 2) { // Shortcut
            return shortcutField.text.length > 0
        }
        return false
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
            iconField.text = selectedFile
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
            popup.deleteAction()
            popup.close()
        }
    }

    TextField {
        id: shortcutField
        visible: false
    }

    signal createAction()
    signal saveAction()
    signal deleteAction()

    function clearFields() {
        nameField.text = ""
        commandField.text = ""
        argumentsField.text = ""
        iconField.text = ""
        shortcutField.text = ""
        typeComboBox.currentIndex = 0
        mediaKeyComboBox.currentIndex = 0

        // Clear shortcut fields
        if (modifier1ComboBox) {
            modifier1ComboBox.currentIndex = 0
        }
        if (modifier2ComboBox) {
            modifier2ComboBox.currentIndex = 0
        }
        if (keyTextField) {
            keyTextField.text = ""
        }

        isModifying = false
    }

    function setFieldsFromAction(name, command, args, icon, type, mediaKey, shortcut) {
        nameField.text = name || ""
        commandField.text = command || ""
        argumentsField.text = args || ""
        iconField.text = icon || ""
        typeComboBox.currentIndex = type || 0
        mediaKeyComboBox.currentIndex = mediaKey || 0

        // Parse and set shortcut when modifying
        if (shortcut && shortcutLayout.parseShortcut) {
            shortcutLayout.parseShortcut(shortcut)
        } else {
            // Clear shortcut fields if no shortcut
            if (modifier1ComboBox) {
                modifier1ComboBox.currentIndex = 0
            }
            if (modifier2ComboBox) {
                modifier2ComboBox.currentIndex = 0
            }
            if (keyTextField) {
                keyTextField.text = ""
            }
        }
    }
}
