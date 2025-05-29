#include "actionpadserver.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

// ActionModel Implementation
ActionModel::ActionModel(QObject *parent) : QAbstractListModel(parent)
{
}

void ActionModel::addAction(const QString &name, const QString &command,
                            const QString &arguments, const QString &icon)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());

    Action action;
    action.id = m_nextId++;
    action.name = name;
    action.command = command;
    action.arguments = arguments;
    action.icon = icon;

    m_actions.append(action);
    endInsertRows();

    // Auto-save after adding
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

void ActionModel::updateAction(int index, const QString &name, const QString &command,
                               const QString &arguments, const QString &icon)
{
    if (index < 0 || index >= m_actions.size())
        return;

    m_actions[index].name = name;
    m_actions[index].command = command;
    m_actions[index].arguments = arguments;
    m_actions[index].icon = icon;

    emit dataChanged(this->index(index), this->index(index));

    // Auto-save after updating
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

            if (action.arguments.isEmpty()) {
                process->start(action.command);
            } else {
                QStringList args = action.arguments.split(' ', Qt::SkipEmptyParts);
                process->start(action.command, args);
            }

            return;
        }
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
