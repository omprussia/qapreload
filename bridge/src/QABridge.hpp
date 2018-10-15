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

    void initializeBootstrap(QTcpSocket *socket, const QVariant &appPackageArg);
    void appConnectBootstrap(QTcpSocket *socket);
    void appDisconnectBootstrap(QTcpSocket* socket);

    void installAppBootstrap(QTcpSocket *socket, const QVariant &appPathArg, const QVariant &, const QVariant &, const QVariant &);
    void activateAppBootstrap(QTcpSocket *socket, const QVariant &appIdArg, const QVariant &, const QVariant &, const QVariant &);
    void terminateAppBootstrap(QTcpSocket *socket, const QVariant &appId, const QVariant &, const QVariant &, const QVariant &);
    void removeAppBootstrap(QTcpSocket *socket, const QVariant &appNameArg, const QVariant &, const QVariant &, const QVariant &);
    void isAppInstalledBootstrap(QTcpSocket *socket, const QVariant &appNameArg);
    void queryAppStateBootstrap(QTcpSocket *socket, const QVariant &appId);
    void pushFileBootstrap(QTcpSocket *socket, const QVariant &pathArg, const QVariant &dataArg);
    void pullFileBootstrap(QTcpSocket *socket, const QVariant &pathArg);
    void lockBootstrap(QTcpSocket *socket, const QVariant &secondsArg);
    void unlockBootstrap(QTcpSocket *socket);
    void isLockedBootstrap(QTcpSocket *socket);
    void launchAppBootstrap(QTcpSocket *socket);
    void closeAppBootstrap(QTcpSocket *socket);
    void getCurrentContextBootstrap(QTcpSocket *socket);
    void getDeviceTimeBootstrap(QTcpSocket *socket, const QVariant &format = QVariant());

    void pullFolderBootstrap(QTcpSocket *socket, const QVariant &pathArg);
    void implicitWaitBootstrap(QTcpSocket *socket, const QVariant &msecondArg);
    void asyncScriptTimeoutBootstrap(QTcpSocket *socket, const QVariant &msecondArg);
    void timeoutsBootstrap(QTcpSocket *socket, const QVariant &, const QVariant &, const QVariant &, const QVariant &msecondArg, const QVariant &);
    void compareImagesBootstrap(QTcpSocket *socket, const QVariant &matchFeatures, const QVariant &firstImage, const QVariant &secondImage, const QVariant &, const QVariant &, const QVariant &);
    void setNetworkConnectionBootstrap(QTcpSocket *socket, const QVariant &connectionType);
    void getNetworkConnectionBootstrap(QTcpSocket *socket);
    void activateIMEEngineBootstrap(QTcpSocket *socket, const QVariant &engine);
    void availableIMEEnginesBootstrap(QTcpSocket *socket);
    void getActiveIMEEngineBootstrap(QTcpSocket *socket);
    void deactivateIMEEngineBootstrap(QTcpSocket *socket);
    void getStringsBootstrap(QTcpSocket *socket, const QVariant &language, const QVariant &stringFile);
    void endCoverageBootstrap(QTcpSocket *socket, const QVariant &intent, const QVariant &path);
    void resetBootstrap(QTcpSocket *socket);
    void mobileShakeBootstrap(QTcpSocket *socket);
    void getSettingsBootstrap(QTcpSocket *socket);
    void getContextsBootstrap(QTcpSocket *socket);
    void getCurrentPackageBootstrap(QTcpSocket *socket);
    void toggleLocationServicesBootstrap(QTcpSocket *socket);
    void openNotificationsBootstrap(QTcpSocket *socket);
    void getGeoLocationBootstrap(QTcpSocket *socket);
    void getLogTypesBootstrap(QTcpSocket *socket);
    void getLogBootstrap(QTcpSocket *socket, const QVariant &typeArg);
    void setGeoLocationBootstrap(QTcpSocket *socket, const QVariant &location);
    void startRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments);
    void stopRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments);

    void ApplicationReady(const QString &appName);

private slots:
    void processCommand(QTcpSocket *socket, const QByteArray &cmd);
    void forwardToApp(QTcpSocket *socket, const QByteArray &data);

private:
    void connectAppSocket(const QString &appName);
    int getNetworkConnection() const;

    void socketReply(QTcpSocket *socket, const QVariant &value, int status = 0);
    bool compressFolder(QString sourceFolder, QString prefex);

    QTcpServer *m_server = nullptr;
    QHash<QTcpSocket*, QString> m_appSocket;
    QHash<QString, int> m_appPort;

    QByteArray m_dataStream;

    QEventLoop* m_connectLoop;
    QString m_connectAppName;
    QABridgeAdaptor *m_adaptor = nullptr;
};

#endif // QASERVICE_HPP
