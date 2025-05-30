pragma Singleton
import QtCore

Settings {
    id: settings
    property int port: 8080
    property bool darkMode: true
    property bool windowVisibleStartup: true
    property bool autostartServer: false
}
