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
    visible: true
    title: qsTr("Action Pad Server")
    Material.theme: Material.Dark
    color: Material.theme === Material.Dark ? "#1C1C1C" : "#E3E3E3"

    header: ToolBar {
        Material.elevation: 6
        Material.background: Material.theme === Material.Dark ? "#2B2B2B" : "#FFFFFF"

        ToolButton {
            icon.source: "qrc:/icons/plus.png"
            icon.width: 18
            icon.height: 18
            anchors.left: parent.left
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
            font.pixelSize: 18
            font.bold: true
            anchors.centerIn: parent
        }

        Row {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 10

            Label {
                text: "Port:"
                anchors.verticalCenter: parent.verticalCenter
            }

            SpinBox {
                id: portSpinBox
                from: 1024
                to: 65535
                value: 8080
                enabled: !ActionPadServer.isRunning
                width: 120
                height: implicitHeight - 6
                anchors.verticalCenter: parent.verticalCenter
            }

            ToolButton {
                icon.source: ActionPadServer.isRunning ? "qrc:/icons/stop.png" : "qrc:/icons/play.png"
                icon.width: 18
                icon.height: 18
                onClicked: {
                    if (ActionPadServer.isRunning) {
                        ActionPadServer.stopServer()
                    } else {
                        ActionPadServer.startServer(portSpinBox.value)
                    }
                }
            }
        }
    }

    ActionDialog {
        id: actionDialog
        anchors.centerIn: parent

        property int modifyingIndex: -1

        onDeleteRequested: {
            ActionPadServer.actionModel.removeAction(modifyingIndex)
            close()
        }

        onAccepted: {
            if (isModifying) {
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
            } else {
                ActionPadServer.actionModel.addAction(
                            actionName,
                            command,
                            commandArgs,
                            icon,
                            actionType,
                            mediaKey,
                            shortcutKey
                            )
            }
            clearFields()
        }
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
                    source: actionButton.model.icon.length > 0 ? actionButton.model.icon : "qrc:/icons/placeholder.png"
                    fillMode: Image.PreserveAspectFit
                    color: Material.theme === Material.Dark ? "white" : "white"
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
