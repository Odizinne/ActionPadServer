import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import Odizinne.ActionPadServer 1.0

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: qsTr("Action Pad Server")

    ActionPadServer {
        id: server

        onClientConnected: function(address) {
            statusText.text += "\nClient connected: " + address
            statusText.text += "\nSent " + server.actionModel.rowCount() + " actions to client"
        }

        onClientDisconnected: function(address) {
            statusText.text += "\nClient disconnected: " + address
        }

        onActionExecuted: function(actionId, success, output) {
            statusText.text += "\nAction " + actionId + " executed: " +
                    (success ? "success" : "failed")
            if (output.length > 0) {
                statusText.text += "\nOutput: " + output
            }
        }
    }

    ActionDialog {
        id: actionDialog
        anchors.centerIn: parent

        onAccepted: {
            server.actionModel.addAction(
                        actionName,
                        command,
                        commandArgs,
                        icon
                        )

            statusText.text += "\nAction '" + actionName + "' added and pushed to " +
                    server.clientCount + " client(s)"

            clearFields()
        }
    }


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        // Server Controls
        GroupBox {
            title: "Server Control"
            Layout.fillWidth: true

            RowLayout {
                anchors.fill: parent

                Button {
                    text: server.isRunning ? "Stop Server" : "Start Server"
                    onClicked: {
                        if (server.isRunning) {
                            server.stopServer()
                        } else {
                            server.startServer(8080)
                        }
                    }
                }

                Label {
                    text: server.isRunning ?
                              "Server running on " + server.serverAddress + ":" + server.serverPort :
                              "Server stopped"
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: "Connected clients: " + server.clientCount
                    color: server.clientCount > 0 ? "green" : "#666"
                }
            }
        }

        // Actions Setup
        GroupBox {
            title: "Actions"
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent

                // New Action Button
                RowLayout {
                    Layout.fillWidth: true

                    Button {
                        text: "New Action"
                        highlighted: true
                        onClicked: actionDialog.open()
                    }

                    Item { Layout.fillWidth: true }

                    Label {
                        text: server.actionModel.rowCount() + " action(s) configured"
                        color: "#666"
                    }
                }

                // Actions List
                ListView {
                    id: actionsList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: server.actionModel

                    delegate: Rectangle {
                        width: actionsList.width
                        height: 80
                        border.color: "#ccc"
                        border.width: 1
                        radius: 4

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10

                            // Icon preview
                            Image {
                                Layout.preferredWidth: 32
                                Layout.preferredHeight: 32
                                source: model.icon.length > 0 ? model.icon : "qrc:/icons/placeholder.png"
                                fillMode: Image.PreserveAspectFit

                                Rectangle {
                                    anchors.fill: parent
                                    color: "transparent"
                                    border.color: "#ddd"
                                    border.width: 1
                                    radius: 2
                                    visible: parent.status === Image.Error

                                    Text {
                                        anchors.centerIn: parent
                                        text: "?"
                                        color: "#999"
                                    }
                                }
                            }

                            Column {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: model.name
                                    font.bold: true
                                    font.pixelSize: 14
                                }
                                Text {
                                    text: "Command: " + model.command +
                                          (model.arguments.length > 0 ? " " + model.arguments : "")
                                    color: "#666"
                                    font.pixelSize: 12
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    text: "Icon: " + (model.icon.length > 0 ? model.icon : "default placeholder")
                                    color: "#999"
                                    font.pixelSize: 11
                                }
                            }

                            Button {
                                text: "Test"
                                onClicked: server.executeAction(model.actionId)
                            }

                            Button {
                                text: "Remove"
                                onClicked: {
                                    var actionName = model.name
                                    server.actionModel.removeAction(index)
                                    statusText.text += "\nAction '" + actionName + "' removed and update pushed to " +
                                            server.clientCount + " client(s)"
                                }
                            }
                        }
                    }
                }
            }
        }

        // Status/Debug Area
        GroupBox {
            title: "Status & Debug"
            Layout.fillWidth: true
            Layout.preferredHeight: 150

            ScrollView {
                anchors.fill: parent

                TextArea {
                    id: statusText
                    text: "Action Pad Server ready..."
                    readOnly: true
                    wrapMode: TextArea.Wrap
                }
            }
        }
    }
}
