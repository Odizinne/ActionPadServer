pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Material
import QtQuick.Controls.impl
import Odizinne.ActionPadServer 1.0

ApplicationWindow {
    id: window
    width: 400
    height: mainCol.implicitHeight + 30
    maximumWidth: 400
    maximumHeight: mainCol.implicitHeight + 30
    minimumWidth: 400
    minimumHeight: mainCol.implicitHeight + 30
    visible: false
    title: qsTr("Action Pad Server")
    Material.theme: UserSettings.darkMode ? Material.Dark : Material.Light
    color: UserSettings.darkMode ? "#1C1C1C" : "#E3E3E3"
    transientParent: null
    property int rowHeight: 40

    ColumnLayout {
        id: mainCol
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Label {
            text: "Server configuration"
            Layout.bottomMargin: -10
            Layout.leftMargin: 5
            color: Material.accent
        }

        Pane {
            Layout.fillWidth: true
            Material.background: UserSettings.darkMode ? "#2B2B2B" : "#FFFFFF"
            Material.elevation: 6
            Material.roundedScale: Material.ExtraSmallScale

            ColumnLayout {
                anchors.fill: parent
                spacing: 14

                RowLayout {
                    Layout.preferredHeight: window.rowHeight
                    Label {
                        text: "Server status"
                        Layout.fillWidth: true
                    }

                    Label {
                        text: ActionPadServer.isRunning ? "Running" : "Stopped"
                        color: ActionPadServer.isRunning ? Material.accent : Material.foreground
                    }
                }

                RowLayout {
                    Layout.preferredHeight: window.rowHeight
                    Label {
                        text: "Port"
                        Layout.fillWidth: true
                    }

                    SpinBox {
                        id: portSpinBox
                        from: 1024
                        to: 65535
                        value: UserSettings.port
                        onValueChanged: UserSettings.port = value
                        enabled: !ActionPadServer.isRunning
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: implicitHeight - 6
                    }
                }

                RowLayout {
                    Layout.preferredHeight: window.rowHeight
                    Label {
                        text: "Autostart server"
                        Layout.fillWidth: true
                    }

                    Switch {
                        checked: UserSettings.autostartServer
                        onClicked: UserSettings.autostartServer = checked
                    }
                }

                RowLayout {
                    Layout.preferredHeight: window.rowHeight
                    Label {
                        text: "Server IP"
                        Layout.fillWidth: true
                    }

                    Label {
                        text: ActionPadServer.serverAddress
                    }
                }
                RowLayout {
                    Layout.preferredHeight: window.rowHeight
                    Button {
                        Material.elevation: 0
                        Material.roundedScale: Material.ExtraSmallScale
                        Layout.fillWidth: true
                        text: ActionPadServer.isRunning ? "Stop server" : "Start server"
                        icon.source: ActionPadServer.isRunning ? "qrc:/icons/stop.png" : "qrc:/icons/play.png"
                        icon.width: 16
                        icon.height: 16
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
        }

        Label {
            text: "Application configuration"
            Layout.bottomMargin: -10
            Layout.leftMargin: 5
            color: Material.accent
        }

        Pane {
            Layout.fillWidth: true
            Material.background: UserSettings.darkMode ? "#2B2B2B" : "#FFFFFF"
            Material.elevation: 6
            Material.roundedScale: Material.ExtraSmallScale

            ColumnLayout {
                anchors.fill: parent
                spacing: 14
                RowLayout {
                    Layout.preferredHeight: window.rowHeight
                    Label {
                        text: "Run at system startup"
                        Layout.fillWidth: true
                    }

                    Switch {
                        checked: ActionPadServer.isRunAtStartup
                        onClicked: ActionPadServer.setRunAtStartup(checked)
                    }
                }

                RowLayout {
                    Layout.preferredHeight: window.rowHeight
                    Label {
                        text: "Show window on app startup"
                        Layout.fillWidth: true
                    }

                    Switch {
                        checked: UserSettings.windowVisibleStartup
                        onClicked: UserSettings.windowVisibleStartup = checked
                    }
                }
                RowLayout {
                    Layout.preferredHeight: window.rowHeight

                    Label {
                        text: "Dark mode"
                        Layout.fillWidth: true
                    }

                    Item {
                        Layout.preferredHeight: 24
                        Layout.preferredWidth: 24

                        IconImage {
                            id: sunImage
                            anchors.fill: parent
                            source: "qrc:/icons/sun.svg"
                            opacity: !themeSwitch.checked ? 1 : 0
                            rotation: themeSwitch.checked ? 360 : 0
                            color: "black"
                            mipmap: true

                            Behavior on rotation {
                                NumberAnimation {
                                    duration: 500
                                    easing.type: Easing.OutQuad
                                }
                            }

                            Behavior on opacity {
                                NumberAnimation { duration: 500 }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: themeSwitch.checked = !themeSwitch.checked
                            }
                        }

                        IconImage {
                            anchors.fill: parent
                            id: moonImage
                            source: "qrc:/icons/moon.svg"
                            opacity: themeSwitch.checked ? 1 : 0
                            rotation: themeSwitch.checked ? 360 : 0
                            color: "white"
                            mipmap: true

                            Behavior on rotation {
                                NumberAnimation {
                                    duration: 500
                                    easing.type: Easing.OutQuad
                                }
                            }

                            Behavior on opacity {
                                NumberAnimation { duration: 100 }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: themeSwitch.checked = !themeSwitch.checked
                            }
                        }
                    }
                    Switch {
                        id: themeSwitch
                        checked: UserSettings.darkMode
                        onClicked: UserSettings.darkMode = checked
                    }
                }
            }
        }
    }
}
