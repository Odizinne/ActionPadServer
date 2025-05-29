#include "actionpadserver.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

// ActionModel Implementation
ActionModel::ActionModel(QObject *parent) : QAbstractListModel(parent)
{
}

ActionPadServer* ActionPadServer::m_instance = nullptr;

ActionPadServer* ActionPadServer::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    if (!m_instance) {
        m_instance = new ActionPadServer();
    }
    return m_instance;
}

ActionPadServer* ActionPadServer::instance()
{
    if (!m_instance) {
        m_instance = new ActionPadServer();
    }
    return m_instance;
}

void ActionModel::addAction(const QString &name, const QString &command,
                            const QString &arguments, const QString &icon,
                            int type, int mediaKey, const QString &shortcut)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    Action action;
    action.id = m_nextId++;
    action.name = name;
    action.command = command;
    action.arguments = arguments;
    action.icon = icon;
    action.type = type;
    action.mediaKey = mediaKey;
    action.shortcut = shortcut;

    m_actions.append(action);
    endInsertRows();

    // Auto-save after adding
    saveToSettings();
    emit actionsChanged();
}

void ActionModel::updateAction(int index, const QString &name, const QString &command,
                               const QString &arguments, const QString &icon,
                               int type, int mediaKey, const QString &shortcut)
{
    if (index < 0 || index >= m_actions.size())
        return;

    m_actions[index].name = name;
    m_actions[index].command = command;
    m_actions[index].arguments = arguments;
    m_actions[index].icon = icon;
    m_actions[index].type = type;
    m_actions[index].mediaKey = mediaKey;
    m_actions[index].shortcut = shortcut;

    emit dataChanged(this->index(index), this->index(index));

    // Auto-save after updating
    saveToSettings();
    emit actionsChanged();
}

void ActionModel::removeAction(int index)
{
    if (index < 0 || index >= m_actions.size())
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_actions.removeAt(index);
    endRemoveRows();

    // Auto-save after removing
    saveToSettings();
    emit actionsChanged();
}

int ActionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_actions.size();
}

QVariant ActionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_actions.size())
        return QVariant();

    const Action &action = m_actions[index.row()];

    switch (role) {
    case IdRole: return action.id;
    case NameRole: return action.name;
    case CommandRole: return action.command;
    case ArgumentsRole: return action.arguments;
    case IconRole: return action.icon;
    case TypeRole: return action.type;           // Add this
    case MediaKeyRole: return action.mediaKey;   // Add this
    case ShortcutRole: return action.shortcut;   // Add this
    }

    return QVariant();
}

QHash<int, QByteArray> ActionModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "actionId";
    roles[NameRole] = "name";
    roles[CommandRole] = "command";
    roles[ArgumentsRole] = "arguments";
    roles[IconRole] = "icon";
    roles[TypeRole] = "type";           // Add this
    roles[MediaKeyRole] = "mediaKey";   // Add this
    roles[ShortcutRole] = "shortcut";   // Add this
    return roles;
}

void ActionModel::loadActions()
{
    loadFromSettings();
}

void ActionModel::saveToSettings()
{
    QSettings settings;
    settings.beginWriteArray("actions");

    for (int i = 0; i < m_actions.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("id", m_actions[i].id);
        settings.setValue("name", m_actions[i].name);
        settings.setValue("command", m_actions[i].command);
        settings.setValue("arguments", m_actions[i].arguments);
        settings.setValue("icon", m_actions[i].icon);
        settings.setValue("type", m_actions[i].type);
        settings.setValue("mediaKey", m_actions[i].mediaKey);
        settings.setValue("shortcut", m_actions[i].shortcut);
    }

    settings.endArray();
    settings.setValue("nextId", m_nextId);
}

void ActionModel::loadFromSettings()
{
    QSettings settings;
    int size = settings.beginReadArray("actions");

    beginResetModel();
    m_actions.clear();

    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        Action action;
        action.id = settings.value("id").toInt();
        action.name = settings.value("name").toString();
        action.command = settings.value("command").toString();
        action.arguments = settings.value("arguments").toString();
        action.icon = settings.value("icon").toString();
        action.type = settings.value("type", 0).toInt();
        action.mediaKey = settings.value("mediaKey", 0).toInt();
        action.shortcut = settings.value("shortcut").toString();
        m_actions.append(action);
    }

    settings.endArray();
    m_nextId = settings.value("nextId", 1).toInt();

    endResetModel();
}
// ActionPadServer Implementation
ActionPadServer::ActionPadServer(QObject *parent) : QObject(parent)
{
    m_server = new QTcpServer(this);
    connect(m_server, &QTcpServer::newConnection, this, &ActionPadServer::onNewConnection);

    // Connect to ActionModel changes to broadcast updates
    connect(&m_actionModel, &ActionModel::actionsChanged, this, &ActionPadServer::broadcastActionsUpdate);

    // Load saved actions on startup
    m_actionModel.loadActions();
}

bool ActionPadServer::startServer(int port)
{
    if (m_server->isListening())
        return true;

    m_serverPort = port;

    if (!m_server->listen(QHostAddress::Any, port)) {
        return false;
    }

    // Get local IP address
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol &&
            address != QHostAddress(QHostAddress::LocalHost)) {
            m_serverAddress = address.toString();
            break;
        }
    }

    emit isRunningChanged();
    emit serverAddressChanged();
    emit serverPortChanged();

    return true;
}

void ActionPadServer::stopServer()
{
    if (!m_server->isListening())
        return;

    // Disconnect all clients
    for (auto client : m_clients) {
        client->disconnectFromHost();
    }
    m_clients.clear();

    m_server->close();
    emit isRunningChanged();
    emit clientCountChanged();
}

void ActionPadServer::executeAction(int actionId)
{
    const auto& actions = m_actionModel.getActions();

    for (const auto& action : actions) {
        if (action.id == actionId) {
            QProcess *process = new QProcess(this);

            connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    [=](int exitCode, QProcess::ExitStatus exitStatus) {
                        QString output = process->readAllStandardOutput();
                        bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);
                        emit actionExecuted(actionId, success, output);
                        process->deleteLater();
                    });

            if (action.type == 0) { // Command
                if (action.arguments.isEmpty()) {
                    process->start(action.command);
                } else {
                    QStringList args = action.arguments.split(' ', Qt::SkipEmptyParts);
                    process->start(action.command, args);
                }
            } else if (action.type == 1) { // Media Key
                executeMediaKey(action.mediaKey);
                process->deleteLater(); // We don't need the process for media keys
            } else if (action.type == 2) { // Shortcut
                executeShortcut(action.shortcut);
                process->deleteLater(); // We don't need the process for shortcuts
            }

            return;
        }
    }
}

// Add these new methods to ActionPadServer class
void ActionPadServer::executeMediaKey(int mediaKeyIndex)
{
#ifdef Q_OS_WIN
    // Windows implementation using keybd_event
    BYTE vkCode = 0;
    switch (mediaKeyIndex) {
    case 0: vkCode = VK_MEDIA_PLAY_PAUSE; break; // Play/Pause
    case 1: vkCode = VK_MEDIA_PLAY_PAUSE; break; // Play (same as play/pause)
    case 2: vkCode = VK_MEDIA_PLAY_PAUSE; break; // Pause (same as play/pause)
    case 3: vkCode = VK_MEDIA_STOP; break;       // Stop
    case 4: vkCode = VK_MEDIA_NEXT_TRACK; break; // Next Track
    case 5: vkCode = VK_MEDIA_PREV_TRACK; break; // Previous Track
    case 6: vkCode = VK_VOLUME_UP; break;        // Volume Up
    case 7: vkCode = VK_VOLUME_DOWN; break;      // Volume Down
    case 8: vkCode = VK_VOLUME_MUTE; break;      // Volume Mute
    }
    if (vkCode) {
        keybd_event(vkCode, 0, KEYEVENTF_EXTENDEDKEY, 0);
        keybd_event(vkCode, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    }
#elif defined(Q_OS_LINUX)
    // Linux implementation using xdotool or playerctl
    QString command;
    switch (mediaKeyIndex) {
    case 0: command = "playerctl play-pause"; break;
    case 1: command = "playerctl play"; break;
    case 2: command = "playerctl pause"; break;
    case 3: command = "playerctl stop"; break;
    case 4: command = "playerctl next"; break;
    case 5: command = "playerctl previous"; break;
    case 6: command = "pactl set-sink-volume @DEFAULT_SINK@ +5%"; break;
    case 7: command = "pactl set-sink-volume @DEFAULT_SINK@ -5%"; break;
    case 8: command = "pactl set-sink-mute @DEFAULT_SINK@ toggle"; break;
    }
    if (!command.isEmpty()) {
        QProcess::startDetached("/bin/sh", QStringList() << "-c" << command);
    }
#endif
}

void ActionPadServer::executeShortcut(const QString &shortcut)
{
#ifdef Q_OS_WIN
    // Windows implementation stays the same
    QProcess::startDetached("powershell", QStringList() << "-Command" <<
                                              QString("Add-Type -AssemblyName System.Windows.Forms; [System.Windows.Forms.SendKeys]::SendWait('%s')")
                                                  .arg(shortcut.replace("Ctrl+", "^").replace("Alt+", "%").replace("Shift+", "+")));
#elif defined(Q_OS_LINUX)
    // Try ydotool first (Wayland), fallback to xdotool (X11)
    QString cmd = shortcut;
    cmd = cmd.replace("Ctrl+", "ctrl+")
              .replace("Alt+", "alt+")
              .replace("Shift+", "shift+")
              .replace("Meta+", "super+");

    // Check if ydotool is available (better for Wayland)
    if (QProcess::execute("which", QStringList() << "ydotool") == 0) {
        QProcess::startDetached("ydotool", QStringList() << "key" << cmd.toLower());
    }
    // Fallback to xdotool (X11 only)
    else if (QProcess::execute("which", QStringList() << "xdotool") == 0) {
        QProcess::startDetached("xdotool", QStringList() << "key" << cmd.toLower());
    }
    // Last resort: try wtype for simple key combinations
    else if (QProcess::execute("which", QStringList() << "wtype") == 0) {
        // wtype has different syntax, this is a simplified conversion
        QString wtypeCmd = cmd.replace("ctrl+", "-M ctrl -k ")
                               .replace("alt+", "-M alt -k ")
                               .replace("shift+", "-M shift -k ");
        QProcess::startDetached("wtype", QStringList() << wtypeCmd.split(" ", Qt::SkipEmptyParts));
    }
#endif
}

void ActionPadServer::onNewConnection()
{
    QTcpSocket *client = m_server->nextPendingConnection();
    m_clients.append(client);

    connect(client, &QTcpSocket::disconnected, this, &ActionPadServer::onClientDisconnected);
    connect(client, &QTcpSocket::readyRead, this, &ActionPadServer::onClientDataReceived);

    emit clientConnected(client->peerAddress().toString());
    emit clientCountChanged();

    // Send current actions to the new client
    sendActionsToClient(client);
}

void ActionPadServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        emit clientDisconnected(client->peerAddress().toString());
        m_clients.removeAll(client);
        emit clientCountChanged();
        client->deleteLater();
    }
}

void ActionPadServer::onClientDataReceived()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    QByteArray data = client->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isObject()) {
        processClientMessage(client, doc.object());
    }
}

void ActionPadServer::broadcastActionsUpdate()
{
    // Send updated actions list to all connected clients
    for (auto client : m_clients) {
        if (client && client->state() == QTcpSocket::ConnectedState) {
            sendActionsToClient(client);
        }
    }
}

void ActionPadServer::sendActionsToClient(QTcpSocket *client)
{
    QJsonObject message;
    message["type"] = "actions";

    QJsonArray actionsArray;
    const auto& actions = m_actionModel.getActions();

    for (const auto& action : actions) {
        QJsonObject actionObj;
        actionObj["id"] = action.id;
        actionObj["name"] = action.name;

        // Use placeholder icon if none provided
        QString iconPath = action.icon.isEmpty() ? "qrc:/icons/placeholder.png" : action.icon;
        actionObj["icon"] = iconPath;

        actionsArray.append(actionObj);
    }

    message["actions"] = actionsArray;

    QJsonDocument doc(message);
    client->write(doc.toJson(QJsonDocument::Compact) + "\n");
}

void ActionPadServer::processClientMessage(QTcpSocket *client, const QJsonObject &message)
{
    QString type = message["type"].toString();

    if (type == "action_press") {
        int actionId = message["actionId"].toInt();
        executeAction(actionId);
    }
    else if (type == "get_actions") {
        sendActionsToClient(client);
    }
}
