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
#include <QSettings>
#include <QQmlEngine>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>

struct Action {
    QString name;
    QString command;
    QString arguments;
    QString icon;
    int id;
    int type = 0;           // 0=command, 1=media, 2=shortcut
    int mediaKey = 0;       // Media key index
    QString shortcut;       // Shortcut string
};

class ActionModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum ActionRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        CommandRole,
        ArgumentsRole,
        IconRole,
        TypeRole,
        MediaKeyRole,
        ShortcutRole
    };

    explicit ActionModel(QObject *parent = nullptr);
    Q_INVOKABLE void addAction(const QString &name, const QString &command,
                               const QString &arguments, const QString &icon,
                               int type = 0, int mediaKey = 0, const QString &shortcut = "");
    Q_INVOKABLE void updateAction(int index, const QString &name, const QString &command,
                                  const QString &arguments, const QString &icon,
                                  int type = 0, int mediaKey = 0, const QString &shortcut = "");
    Q_INVOKABLE void removeAction(int index);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    const QList<Action>& getActions() const { return m_actions; }
    void loadActions();

signals:
    void actionsChanged();

private:
    void saveToSettings();
    void loadFromSettings();

    QList<Action> m_actions;
    int m_nextId = 1;
};

class ActionPadServer : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY isRunningChanged)
    Q_PROPERTY(QString serverAddress READ serverAddress NOTIFY serverAddressChanged)
    Q_PROPERTY(int serverPort READ serverPort NOTIFY serverPortChanged)
    Q_PROPERTY(int clientCount READ clientCount NOTIFY clientCountChanged)
    Q_PROPERTY(ActionModel* actionModel READ actionModel CONSTANT)
    Q_PROPERTY(bool windowVisible READ windowVisible WRITE setWindowVisible NOTIFY windowVisibleChanged)
    Q_PROPERTY(bool isRunAtStartup READ isRunAtStartup NOTIFY isRunAtStartupChanged FINAL)

public:
    static ActionPadServer* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    static ActionPadServer* instance();

    bool isRunning() const { return m_server->isListening(); }
    QString serverAddress() const { return m_serverAddress; }
    int serverPort() const { return m_serverPort; }
    int clientCount() const { return m_clients.size(); }
    ActionModel* actionModel() { return &m_actionModel; }
    bool windowVisible() const { return m_windowVisible; }
    void setWindowVisible(bool visible);
    bool isRunAtStartup() const { return m_isRunAtStartup; }

    Q_INVOKABLE bool startServer(int port = 8080);
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE void executeAction(int actionId);
    Q_INVOKABLE void showSettings();
    Q_INVOKABLE void setRunAtStartup(bool enable);

signals:
    void isRunningChanged();
    void serverAddressChanged();
    void serverPortChanged();
    void clientCountChanged();
    void windowVisibleChanged();
    void clientConnected(const QString &address);
    void clientDisconnected(const QString &address);
    void actionExecuted(int actionId, bool success, const QString &output);
    void showWindow();
    void hideWindow();
    void settingsRequested();
    void isRunAtStartupChanged();

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onClientDataReceived();
    void broadcastActionsUpdate();
    void toggleWindowVisibility();
    void exitApplication();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    explicit ActionPadServer(QObject *parent = nullptr);
    void sendActionsToClient(QTcpSocket *client);
    void processClientMessage(QTcpSocket *client, const QJsonObject &message);
    void createTrayMenu();
    void setupSystemTray();

    static ActionPadServer* m_instance;
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
    ActionModel m_actionModel;
    QString m_serverAddress;
    int m_serverPort = 8080;
    bool m_windowVisible = true;
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_showHideAction;
    QAction *m_settingsAction;
    QAction *m_exitAction;
    bool m_isRunAtStartup{false};

    void executeMediaKey(int mediaKeyIndex);
    void executeShortcut(const QString &shortcut);
};

#endif // ACTIONPADSERVER_H
