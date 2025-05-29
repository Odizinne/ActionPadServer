#include "actionpadserver.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <windows.h>
#include <QFile>
#include <QFileInfo>

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
                    [=, this](int exitCode, QProcess::ExitStatus exitStatus) {
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
    BYTE vkCode = 0;
    switch (mediaKeyIndex) {
    case 0: vkCode = VK_MEDIA_PLAY_PAUSE; break; // Play/Pause
    case 1: vkCode = VK_MEDIA_STOP; break;       // Stop
    case 2: vkCode = VK_MEDIA_NEXT_TRACK; break; // Next Track
    case 3: vkCode = VK_MEDIA_PREV_TRACK; break; // Previous Track
    case 4: vkCode = VK_VOLUME_UP; break;        // Volume Up
    case 5: vkCode = VK_VOLUME_DOWN; break;      // Volume Down
    case 6: vkCode = VK_VOLUME_MUTE; break;      // Volume Mute
    }
    if (vkCode) {
        keybd_event(vkCode, 0, KEYEVENTF_EXTENDEDKEY, 0);
        keybd_event(vkCode, 0, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    }
}

void ActionPadServer::executeShortcut(const QString &shortcut)
{
    QStringList parts = shortcut.split('+', Qt::SkipEmptyParts);

    QList<WORD> keysToPress;

    for (const QString &part : parts) {
        QString key = part.trimmed();
        WORD vkCode = 0;

        // Handle modifiers
        if (key == "Ctrl") {
            vkCode = VK_CONTROL;
        }
        else if (key == "Alt") {
            vkCode = VK_MENU;
        }
        else if (key == "Shift") {
            vkCode = VK_SHIFT;
        }
        else if (key == "Meta") {
            vkCode = VK_LWIN; // Left Windows key
        }
        // Handle special keys
        else if (key == "Tab") {
            vkCode = VK_TAB;
        }
        else if (key == "Delete" || key == "Del") {
            vkCode = VK_DELETE;
        }
        else if (key == "Return" || key == "Enter") {
            vkCode = VK_RETURN;
        }
        else if (key == "Escape") {
            vkCode = VK_ESCAPE;
        }
        else if (key == "Space") {
            vkCode = VK_SPACE;
        }
        else if (key == "Home") {
            vkCode = VK_HOME;
        }
        else if (key == "End") {
            vkCode = VK_END;
        }
        else if (key == "Page Up") {
            vkCode = VK_PRIOR;
        }
        else if (key == "Page Down") {
            vkCode = VK_NEXT;
        }
        else if (key == "Up") {
            vkCode = VK_UP;
        }
        else if (key == "Down") {
            vkCode = VK_DOWN;
        }
        else if (key == "Left") {
            vkCode = VK_LEFT;
        }
        else if (key == "Right") {
            vkCode = VK_RIGHT;
        }
        else if (key == "Backspace") {
            vkCode = VK_BACK;
        }
        else if (key == "Insert") {
            vkCode = VK_INSERT;
        }
        // Handle function keys
        else if (key.startsWith("F") && key.length() <= 3) {
            bool ok;
            int fNum = key.mid(1).toInt(&ok);
            if (ok && fNum >= 1 && fNum <= 12) {
                vkCode = VK_F1 + (fNum - 1);
            }
        }
        // Handle single characters and numbers
        else if (key.length() == 1) {
            QChar c = key.at(0).toUpper();
            if (c >= 'A' && c <= 'Z') {
                vkCode = c.unicode(); // A-Z have the same values as VK codes
            }
            else if (c >= '0' && c <= '9') {
                vkCode = c.unicode(); // 0-9 have the same values as VK codes
            }
        }

        if (vkCode != 0) {
            keysToPress.append(vkCode);
        }
    }

    // Press all keys down in order
    for (WORD vk : keysToPress) {
        keybd_event(vk, 0, 0, 0);
    }

    // Small delay to ensure keys are registered
    Sleep(10);

    // Release all keys in reverse order
    for (int i = keysToPress.size() - 1; i >= 0; --i) {
        keybd_event(keysToPress[i], 0, KEYEVENTF_KEYUP, 0);
    }
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
        QString iconData = "placeholder";

        if (!action.icon.isEmpty() && action.icon != "placeholder") {
            if (action.icon.startsWith("qrc:/")) {
                iconData = action.icon;
            } else {
                QUrl iconUrl(action.icon);
                QString filePath = iconUrl.toLocalFile();

                if (filePath.isEmpty()) {
                    filePath = action.icon;
                }

                // Check file size first, before reading
                QFileInfo fileInfo(filePath);
                if (fileInfo.exists() && fileInfo.size() <= 200000) { // 200KB limit
                    QFile iconFile(filePath);
                    if (iconFile.open(QIODevice::ReadOnly)) {
                        QByteArray imageData = iconFile.readAll();
                        QString base64 = imageData.toBase64();

                        QString mimeType = "image/png";
                        if (filePath.endsWith(".jpg", Qt::CaseInsensitive) ||
                            filePath.endsWith(".jpeg", Qt::CaseInsensitive)) {
                            mimeType = "image/jpeg";
                        } else if (filePath.endsWith(".svg", Qt::CaseInsensitive)) {
                            mimeType = "image/svg+xml";
                        } else if (filePath.endsWith(".gif", Qt::CaseInsensitive)) {
                            mimeType = "image/gif";
                        } else if (filePath.endsWith(".ico", Qt::CaseInsensitive)) {
                            mimeType = "image/x-icon";
                        }

                        iconData = QString("data:%1;base64,%2").arg(mimeType, base64);
                    }
                }
                // If file doesn't exist, is too large, or fails to open -> iconData stays "placeholder"
            }
        }

        actionObj["icon"] = iconData;
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
