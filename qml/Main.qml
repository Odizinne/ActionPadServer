pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material
import QtQuick.Controls.impl
import Odizinne.ActionPadServer 1.0

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: UserSettings.windowVisibleStartup
    title: qsTr("Action Pad Server")
    Material.theme: UserSettings.darkMode ? Material.Dark : Material.Light
    color: UserSettings.darkMode ? "#1C1C1C" : "#E3E3E3"

    onVisibleChanged: {
        if (ActionPadServer.windowVisible !== visible) {
            ActionPadServer.windowVisible = visible
        }
    }

    Connections {
        target: ActionPadServer

        function onShowWindow() {
            window.show()
        }

        function onHideWindow() {
            window.close()
        }

        function onSettingsRequested() {
            settingsWindow.visible = !settingsWindow.visible
        }
    }

    header: ToolBar {
        Material.elevation: 6
        Material.background: UserSettings.darkMode ? "#2B2B2B" : "#FFFFFF"

        ToolButton {
            icon.source: "qrc:/icons/plus.png"
            icon.width: 18
            icon.height: 18
            anchors.left: parent.left
            icon.color: UserSettings.darkMode ? "white" : "black"
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                actionDialog.isModifying = false
                actionDialog.clearFields()
                actionDialog.open()
            }
        }

        Label {
            id: serverStatusLabel
            text: ActionPadServer.isRunning ? "Server Running" : "Server Stopped"
            color: ActionPadServer.isRunning ? Material.accent : (UserSettings.darkMode ? "white" : "black")
            font.pixelSize: 18
            font.bold: true
            anchors.centerIn: parent
        }

        ToolButton {
            icon.source: "qrc:/icons/settings.svg"
            icon.width: 18
            icon.height: 18
            anchors.right: parent.right
            icon.color: UserSettings.darkMode ? "white" : "black"
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                settingsWindow.show()
            }
        }
    }

    ActionDialog {
        id: actionDialog

        property int modifyingIndex: -1

        onCreateAction: {
            ActionPadServer.actionModel.addAction(
                actionName,
                command,
                commandArgs,
                icon,
                actionType,
                mediaKey,
                shortcutKey
            )
            clearFields()
        }

        onSaveAction: {
            ActionPadServer.actionModel.updateAction(
                modifyingIndex,
                actionName,
                command,
                commandArgs,
                icon,
                actionType,
                mediaKey,
                shortcutKey
            )
            clearFields()
        }

        onDeleteAction: {
            ActionPadServer.actionModel.removeAction(modifyingIndex)
            clearFields()
        }
    }

    SettingsWindow {
        id: settingsWindow
    }

    GridView {
        id: actionsGrid
        anchors.fill: parent
        anchors.margins: 20
        model: ActionPadServer.actionModel
        cellWidth: 120
        cellHeight: 120
        delegate: Button {
            id: actionButton
            width: actionsGrid.cellWidth - 10
            height: actionsGrid.cellHeight - 10
            Material.roundedScale: Material.SmallScale

            required property var model
            required property int index
            onClicked: {
                actionDialog.isModifying = true
                actionDialog.modifyingIndex = index
                actionDialog.setFieldsFromAction(
                    model.name,
                    model.command || "",
                    model.arguments || "",
                    model.icon || "",
                    model.type || 0,
                    model.mediaKey || 0,
                    model.shortcut || ""
                )
                actionDialog.open()
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                IconImage {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48
                    source: actionButton.model.icon && actionButton.model.icon.length > 0 ?
                            actionButton.model.icon : "qrc:/icons/placeholder.png"
                    fillMode: Image.PreserveAspectFit
                    color: (!actionButton.model.icon || actionButton.model.icon.length === 0) ?
                           (UserSettings.darkMode ? "white" : "black") : "transparent"
                }

                Label {
                    Layout.fillWidth: true
                    text: actionButton.model.name
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    font.pixelSize: 12
                }
            }
        }
    }
}
