#ifndef QASERVICE_HPP
#define QASERVICE_HPP

#include <QObject>
#include <QHash>
#include <QVariant>

class QABridgeAdaptor;
class QEventLoop;
class QTcpServer;
class QTcpSocket;
class QABridge : public QObject
{
    Q_OBJECT
public:
    explicit QABridge(QObject *parent = nullptr);

    enum NetworkConnection {
        NetworkConnectionNone = 0,
        NetworkConnectionAirplane = 1,
        NetworkConnectionWifi = 2,
        NetworkConnectionData = 4,
        NetworkConnectionAll = 6
    };
    Q_ENUM(NetworkConnection)

public slots:
    void start();

private slots:
    void newConnection();

    void readSocket();
    void removeSocket();

    void initializeBootstrap(QTcpSocket *socket, const QString &appName);
    void appConnectBootstrap(QTcpSocket *socket);
    void appDisconnectBootstrap(QTcpSocket *socket, bool autoLaunch);

    void startActivityBootstrap(QTcpSocket *socket, const QString &appName, const QStringList &params);

    void installAppBootstrap(QTcpSocket *socket, const QString &appPath);
    void activateAppBootstrap(QTcpSocket *socket, const QString &appId);
    void terminateAppBootstrap(QTcpSocket *socket, const QString &appId);
    void removeAppBootstrap(QTcpSocket *socket, const QString &appName);
    void isAppInstalledBootstrap(QTcpSocket *socket, const QString &rpmName);
    void queryAppStateBootstrap(QTcpSocket *socket, const QString &appName);
    void pushFileBootstrap(QTcpSocket *socket, const QString &path, const QString &data);
    void pullFileBootstrap(QTcpSocket *socket, const QString &path);
    void lockBootstrap(QTcpSocket *socket, double seconds);
    void unlockBootstrap(QTcpSocket *socket);
    void isLockedBootstrap(QTcpSocket *socket);
    void launchAppBootstrap(QTcpSocket *socket);
    void closeAppBootstrap(QTcpSocket *socket);
    void getCurrentContextBootstrap(QTcpSocket *socket);
    void getDeviceTimeBootstrap(QTcpSocket *socket, const QString &dateFormat = QString());
    void setNetworkConnectionBootstrap(QTcpSocket *socket, double connectionType);
    void getNetworkConnectionBootstrap(QTcpSocket *socket);
    void resetBootstrap(QTcpSocket *socket);
    void mobileShakeBootstrap(QTcpSocket *socket);
    void getSettingsBootstrap(QTcpSocket *socket);
    void getContextsBootstrap(QTcpSocket *socket);
    void getCurrentPackageBootstrap(QTcpSocket *socket);
    void toggleLocationServicesBootstrap(QTcpSocket *socket);
    void openNotificationsBootstrap(QTcpSocket *socket);
    void getGeoLocationBootstrap(QTcpSocket *socket);
    void getLogTypesBootstrap(QTcpSocket *socket);
    void getLogBootstrap(QTcpSocket *socket, const QString &type);
    void setGeoLocationBootstrap(QTcpSocket *socket, const QVariant &location);
    void startRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments);
    void stopRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments);

    void executeBootstrap(QTcpSocket *socket, const QString &command, const QVariant &paramsArg);
    void executeAsyncBootstrap(QTcpSocket *socket, const QString &command, const QVariant &paramsArg);

    void executeCommand_shell(QTcpSocket *socket, const QVariant &executableArg, const QVariant &paramsArg);

    void ApplicationReady(const QString &appName);
    void ApplicationClose(const QString &appName);

private slots:
    void processCommand(QTcpSocket *socket, const QByteArray &cmd);
    void forwardToApp(QTcpSocket *socket, const QByteArray &data);
    void forwardToApp(QTcpSocket *socket, const QString &action, const QVariant &params);

private:
    static bool launchApp(const QString &appName, const QStringList &arguments);
    QByteArray sendToAppSocket(const QString &appName, const QByteArray &data);
    void connectAppSocket(const QString &appName);
    int getNetworkConnection() const;
    bool isAppInstalled(const QString &rpmName);

    void socketReply(QTcpSocket *socket, const QVariant &value, int status = 0);

    QTcpServer *m_server = nullptr;
    QHash<QTcpSocket*, QString> m_appSocket;
    QHash<QString, int> m_appPort;

    QByteArray m_dataStream;

    QEventLoop *m_connectLoop;
    QString m_connectAppName;
    QABridgeAdaptor *m_adaptor = nullptr;
};

#endif // QASERVICE_HPP
