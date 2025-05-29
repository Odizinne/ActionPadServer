#ifndef ACTIONPADSERVER_H
#define ACTIONPADSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QProcess>
#include <QAbstractListModel>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

struct Action {
    QString name;
    QString command;
    QString arguments;
    QString icon;
    int id;
};

class ActionModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ActionRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        CommandRole,
        ArgumentsRole,
        IconRole
    };

    explicit ActionModel(QObject *parent = nullptr);
    Q_INVOKABLE void addAction(const QString &name, const QString &command,
                               const QString &arguments, const QString &icon);
    Q_INVOKABLE void removeAction(int index);
    Q_INVOKABLE void updateAction(int index, const QString &name, const QString &command,
                                  const QString &arguments, const QString &icon);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    const QList<Action>& getActions() const { return m_actions; }

signals:
    void actionsChanged();

private:
    QList<Action> m_actions;
    int m_nextId = 1;
};

class ActionPadServer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(QString serverAddress READ serverAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(int serverPort READ serverPort NOTIFY serverPortChanged)
    Q_PROPERTY(int clientCount READ clientCount NOTIFY clientCountChanged)
    Q_PROPERTY(ActionModel* actionModel READ actionModel CONSTANT)

public:
    explicit ActionPadServer(QObject *parent = nullptr);

    bool isRunning() const { return m_server->isListening(); }
    QString serverAddress() const { return m_serverAddress; }
    int serverPort() const { return m_serverPort; }
    int clientCount() const { return m_clients.size(); }
    ActionModel* actionModel() { return &m_actionModel; }

    Q_INVOKABLE bool startServer(int port = 8080);
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE void executeAction(int actionId);

signals:
    void isRunningChanged();
    void serverAddressChanged();
    void serverPortChanged();
    void clientCountChanged();
    void clientConnected(const QString &address);
    void clientDisconnected(const QString &address);
    void actionExecuted(int actionId, bool success, const QString &output);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onClientDataReceived();
    void broadcastActionsUpdate();

private:
    void sendActionsToClient(QTcpSocket *client);
    void processClientMessage(QTcpSocket *client, const QJsonObject &message);

    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
    ActionModel m_actionModel;
    QString m_serverAddress;
    int m_serverPort = 8080;
};

#endif // ACTIONPADSERVER_H
