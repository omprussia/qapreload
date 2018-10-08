#ifndef QASERVICE_HPP
#define QASERVICE_HPP

#include <QObject>
#include <QVariant>

class QTcpServer;
class QTcpSocket;
class QABridge : public QObject
{
    Q_OBJECT
public:
    explicit QABridge(QObject *parent = nullptr);

public slots:
    void start();

private slots:
    void newConnection();

    void readSocket();
    void removeSocket();

    void appConnectBootstrap(QTcpSocket *socket, const QVariant &arguments);
    void activateAppBootstrap(QTcpSocket *socket, const QVariant &appId, const QVariant &, const QVariant &, const QVariant &);
    void terminateAppBootstrap(QTcpSocket *socket, const QVariant &appId, const QVariant &, const QVariant &, const QVariant &);
    void installAppBootstrap(QTcpSocket *socket, const QVariant &appPath, const QVariant &, const QVariant &, const QVariant &);
    void removeAppBootstrap(QTcpSocket *socket, const QVariant &appName, const QVariant &, const QVariant &, const QVariant &);
    void isAppInstalledBootstrap(QTcpSocket *socket, const QVariant &appName);
    void queryAppStateBootstrap(QTcpSocket *socket, const QVariant &appId);
    void backgroundBootstrap(QTcpSocket *socket, const QVariant &second);
    void setNetworkConnectionBootstrap(QTcpSocket *socket, const QVariant &connectionType);
    void getNetworkConnectionBootstrap(QTcpSocket *socket);
    void activateIMEEngineBootstrap(QTcpSocket *socket, const QVariant &engine);
    void getStringsBootstrap(QTcpSocket *socket, const QVariant &language, const QVariant &stringFile);
    void endCoverageBootstrap(QTcpSocket *socket, const QVariant &intent, const QVariant &path);
    void getClipboardBootstrap(QTcpSocket *socket, const QVariant &contentType);
    void lockBootstrap(QTcpSocket *socket, const QVariant &seconds);
    void executeBootstrap(QTcpSocket *socket, const QVariant &scriptArg, const QVariant &paramsArg);
    void executeAsyncBootstrap(QTcpSocket *socket, const QVariant &scriptArg, const QVariant &paramsArg);
    void pushFileBootstrap(QTcpSocket *socket, const QVariant &pathArg, const QVariant &dataArg);
    void pullFileBootstrap(QTcpSocket *socket, const QVariant &pathArg);
    void pullFolderBootstrap(QTcpSocket *socket, const QVariant &pathArg);
    void touchIdBootstrap(QTcpSocket *socket, const QVariant &matchArg);
    void toggleEnrollTouchIdBootstrap(QTcpSocket *socket, const QVariant &);
    void implicitWaitBootstrap(QTcpSocket *socket, const QVariant &msecondArg);
    void asyncScriptTimeoutBootstrap(QTcpSocket *socket, const QVariant &msecondArg);
    void timeoutsBootstrap(QTcpSocket *socket, const QVariant &, const QVariant &, const QVariant &, const QVariant &msecondArg, const QVariant &);
    void compareImagesBootstrap(QTcpSocket *socket, const QVariant &matchFeatures, const QVariant &firstImage, const QVariant &secondImage, const QVariant &, const QVariant &, const QVariant &);
    void unlockBootstrap(QTcpSocket *socket);
    void isLockedBootstrap(QTcpSocket *socket);
    void launchAppBootstrap(QTcpSocket *socket);
    void closeAppBootstrap(QTcpSocket *socket);
    void resetBootstrap(QTcpSocket *socket);
    void availableIMEEnginesBootstrap(QTcpSocket *socket);
    void getActiveIMEEngineBootstrap(QTcpSocket *socket);
    void deactivateIMEEngineBootstrap(QTcpSocket *socket);
    void mobileShakeBootstrap(QTcpSocket *socket);
    void getSettingsBootstrap(QTcpSocket *socket);
    void getCurrentContextBootstrap(QTcpSocket *socket);
    void getContextsBootstrap(QTcpSocket *socket);
    void getCurrentPackageBootstrap(QTcpSocket *socket);
    void toggleLocationServicesBootstrap(QTcpSocket *socket);
    void openNotificationsBootstrap(QTcpSocket *socket);
    void getGeoLocationBootstrap(QTcpSocket *socket);
    void getLogTypesBootstrap(QTcpSocket *socket);
    void getLogBootstrap(QTcpSocket *socket, const QVariant &typeArg);
    void setGeoLocationBootstrap(QTcpSocket *socket, const QVariant &location);
    void getDeviceTimeBootstrap(QTcpSocket *socket, const QVariant &format = QVariant());
    void startRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments);
    void stopRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments);

private:
    void forwardToApp(QTcpSocket *socket, const QByteArray &data);
    void socketReply(QTcpSocket *socket, const QVariant &value, int status = 0);

    QTcpServer *m_server = nullptr;

};

#endif // QASERVICE_HPP
