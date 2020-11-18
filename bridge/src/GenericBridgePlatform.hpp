// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include "IBridgePlatform.hpp"
#include <QObject>

class QEventLoop;
class GenericBridgePlatform : public IBridgePlatform
{
    Q_OBJECT
public:
    explicit GenericBridgePlatform(QObject *parent);

    virtual void appConnect(QTcpSocket *socket, const QString &appName) override;
    virtual void appReply(QTcpSocket *socket, const QByteArray &cmd) override;

    void removeClient(QTcpSocket *socket) override;

private:
    void execute(QTcpSocket *socket, const QString &methodName, const QVariantList &paramsArg);

private slots:
    virtual void initializeCommand(QTcpSocket *socket, const QString &appName) override;
    virtual void appConnectCommand(QTcpSocket *socket) override;
    virtual void appDisconnectCommand(QTcpSocket *socket, bool autoLaunch) override;
    virtual void startActivityCommand(QTcpSocket *socket, const QString &appName, const QVariantList &params) override;
    virtual void installAppCommand(QTcpSocket *socket, const QString &appPath) override;
    virtual void activateAppCommand(QTcpSocket *socket, const QString &appId) override;
    virtual void terminateAppCommand(QTcpSocket *socket, const QString &appId) override;
    virtual void removeAppCommand(QTcpSocket *socket, const QString &appName) override;
    virtual void isAppInstalledCommand(QTcpSocket *socket, const QString &rpmName) override;
    virtual void queryAppStateCommand(QTcpSocket *socket, const QString &appName) override;
    virtual void pushFileCommand(QTcpSocket *socket, const QString &path, const QString &data) override;
    virtual void pullFileCommand(QTcpSocket *socket, const QString &path) override;
    virtual void lockCommand(QTcpSocket *socket, double seconds) override;
    virtual void unlockCommand(QTcpSocket *socket) override;
    virtual void isLockedCommand(QTcpSocket *socket) override;
    virtual void launchAppCommand(QTcpSocket *socket) override;
    virtual void closeAppCommand(QTcpSocket *socket) override;
    virtual void getCurrentContextCommand(QTcpSocket *socket) override;
    virtual void getDeviceTimeCommand(QTcpSocket *socket, const QString &dateFormat) override;
    virtual void setNetworkConnectionCommand(QTcpSocket *socket, double connectionType) override;
    virtual void getNetworkConnectionCommand(QTcpSocket *socket) override;
    virtual void resetCommand(QTcpSocket *socket) override;
    virtual void mobileShakeCommand(QTcpSocket *socket) override;
    virtual void getSettingsCommand(QTcpSocket *socket) override;
    virtual void getContextsCommand(QTcpSocket *socket) override;
    virtual void getCurrentPackageCommand(QTcpSocket *socket) override;
    virtual void toggleLocationServicesCommand(QTcpSocket *socket) override;
    virtual void openNotificationsCommand(QTcpSocket *socket) override;
    virtual void getGeoLocationCommand(QTcpSocket *socket) override;
    virtual void getLogTypesCommand(QTcpSocket *socket) override;
    virtual void getLogCommand(QTcpSocket *socket, const QString &type) override;
    virtual void setGeoLocationCommand(QTcpSocket *socket, const QVariant &location) override;
    virtual void startRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments) override;
    virtual void stopRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments) override;
    virtual void executeCommand(QTcpSocket *socket, const QString &command, const QVariant &paramsArg) override;
    virtual void executeAsyncCommand(QTcpSocket *socket, const QString &command, const QVariant &paramsArg) override;

// GenericBridgePlatform slots
    void executeCommand_system_shell(QTcpSocket *socket, const QVariant &executableArg, const QVariant &paramsArg);

    void forwardToApp(QTcpSocket *socket, const QByteArray &data);
    void forwardToApp(QTcpSocket *socket, const QString &appName, const QByteArray &data);
    void forwardToApp(QTcpSocket *socket, const QString &action, const QVariant &params);
    QByteArray sendToAppSocket(const QString &appName, const QByteArray &data);

protected:
    virtual bool lauchAppPlatform(QTcpSocket *socket) = 0;
    virtual bool lauchAppStandalone(const QString &appName, const QStringList &arguments = {}) = 0;
    void socketReply(QTcpSocket *socket, const QVariant &value, int status = 0);
    QByteArray actionData(const QString &action, const QVariant &params);

    QHash<QTcpSocket*, QString> m_socketAppName;
    QHash<QString, QTcpSocket*> m_applicationSocket;
    QHash<QTcpSocket*, QString> m_clientFullPath;
    QEventLoop *m_connectLoop;
};

