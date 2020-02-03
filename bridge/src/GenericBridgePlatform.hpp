#pragma once

#include "IBridgePlatform.hpp"
#include <QObject>

class GenericBridgePlatform : public IBridgePlatform
{
    Q_OBJECT
public:
    explicit GenericBridgePlatform(QObject *parent);

signals:
    void applicationReply(QTcpSocket *socket, const QString &appName, const QByteArray &data);

private slots:
//    void initializeCommand(QTcpSocket *socket, const QString &appName) override;
    void appConnectCommand(QTcpSocket *socket) override;
    void appDisconnectCommand(QTcpSocket *socket, bool autoLaunch) override;
//    void startActivityCommand(QTcpSocket *socket, const QString &appName, const QVariantList &params) override;
//    void installAppCommand(QTcpSocket *socket, const QString &appPath) override;
//    void activateAppCommand(QTcpSocket *socket, const QString &appId) override;
//    void terminateAppCommand(QTcpSocket *socket, const QString &appId) override;
//    void removeAppCommand(QTcpSocket *socket, const QString &appName) override;
//    void isAppInstalledCommand(QTcpSocket *socket, const QString &rpmName) override;
//    void queryAppStateCommand(QTcpSocket *socket, const QString &appName) override;
    void pushFileCommand(QTcpSocket *socket, const QString &path, const QString &data) override;
    void pullFileCommand(QTcpSocket *socket, const QString &path) override;
//    void lockCommand(QTcpSocket *socket, double seconds) override;
//    void unlockCommand(QTcpSocket *socket) override;
//    void isLockedCommand(QTcpSocket *socket) override;
//    void launchAppCommand(QTcpSocket *socket) override;
//    void closeAppCommand(QTcpSocket *socket) override;
//    void getCurrentContextCommand(QTcpSocket *socket) override;
    void getDeviceTimeCommand(QTcpSocket *socket, const QString &dateFormat) override;
//    void setNetwokConnectionCommand(QTcpSocket *socket, double connectionType) override;
//    void getNetworkConnectionCommand(QTcpSocket *socket) override;
    void resetCommand(QTcpSocket *socket) override;
    void mobileShakeCommand(QTcpSocket *socket) override;
    void getSettingsCommand(QTcpSocket *socket) override;
    void getContextsCommand(QTcpSocket *socket) override;
    void getCurrentPackageCommand(QTcpSocket *socket) override;
    void toggleLocationServicesCommand(QTcpSocket *socket) override;
    void openNotificationsCommand(QTcpSocket *socket) override;
    void getGeoLocationCommand(QTcpSocket *socket) override;
    void getLogTypesCommand(QTcpSocket *socket) override;
    void getLogCommand(QTcpSocket *socket, const QString &type) override;
    void setGeoLocationCommand(QTcpSocket *socket, const QVariant &location) override;
    virtual void startRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments) override;
    virtual void stopRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments) override;
    void executeCommand(QTcpSocket *socket, const QString &command, const QVariant &paramsArg) override;
    void executeAsyncCommand(QTcpSocket *socket, const QString &command, const QVariant &paramsArg) override;

protected:
    void socketReply(QTcpSocket *socket, const QVariant &value, int status = 0);
    void forwardToApp(QTcpSocket *socket, const QByteArray &data);
    void forwardToApp(QTcpSocket *socket, const QString &appName, const QByteArray &data);
    void forwardToApp(QTcpSocket *socket, const QString &action, const QVariant &params);
    QByteArray sendToAppSocket(const QString &appName, const QByteArray &data);

    QByteArray actionData(const QString &action, const QVariant &params);

    QHash<QTcpSocket*, QString> m_socketAppName;
    QHash<QString, QTcpSocket*> m_applicationSocket;
};

