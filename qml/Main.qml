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
            text: server.isRunning ? "Server Running" : "Server Stopped"
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
                enabled: !server.isRunning
                width: 120
                height: implicitHeight - 6
                anchors.verticalCenter: parent.verticalCenter
            }

            ToolButton {
                icon.source: server.isRunning ? "qrc:/icons/stop.png" : "qrc:/icons/play.png"
                icon.width: 18
                icon.height: 18
                onClicked: {
                    if (server.isRunning) {
                        server.stopServer()
                    } else {
                        server.startServer(portSpinBox.value)
                    }
                }
            }
        }
    }

    ActionPadServer {
        id: server

        onClientConnected: function(address) {
            console.log("Client connected:", address)
        }

        onClientDisconnected: function(address) {
            console.log("Client disconnected:", address)
        }

        onActionExecuted: function(actionId, success, output) {
            console.log("Action", actionId, "executed:", success ? "success" : "failed")
        }
    }

    ActionDialog {
        id: actionDialog
        anchors.centerIn: parent

        property int modifyingIndex: -1

        onDeleteRequested: {
            server.actionModel.removeAction(modifyingIndex)
            close()
        }

        onAccepted: {
            if (isModifying) {
                server.actionModel.updateAction(
                    modifyingIndex,
                    actionName,
                    command,
                    commandArgs,
                    icon
                )
            } else {
                server.actionModel.addAction(
                    actionName,
                    command,
                    commandArgs,
                    icon
                )
            }
            clearFields()
        }
    }

    GridView {
        id: actionsGrid
        anchors.fill: parent
        anchors.margins: 20

        model: server.actionModel
        cellWidth: 120
        cellHeight: 120

        delegate: Button {
            width: actionsGrid.cellWidth - 10
            height: actionsGrid.cellHeight - 10
            Material.roundedScale: Material.SmallScale
            onClicked: {
                actionDialog.isModifying = true
                actionDialog.modifyingIndex = index
                actionDialog.actionName = model.name
                actionDialog.command = model.command
                actionDialog.commandArgs = model.arguments
                actionDialog.icon = model.icon
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
                        source: model.icon.length > 0 ? model.icon : "qrc:/icons/placeholder.png"
                        fillMode: Image.PreserveAspectFit
                        color: Material.theme === Material.Dark ? "white" : "white"
                    }

                    Label {
                        Layout.fillWidth: true
                        text: model.name
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        font.pixelSize: 12
                    }
                }
            }
        }
    }

