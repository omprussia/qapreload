// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include "IBridgePlatform.hpp"
#include "QABridge.hpp"
#include <QObject>

class QABridge;
class QEventLoop;
class GenericBridgePlatform : public IBridgePlatform
{
    Q_OBJECT
public:
    explicit GenericBridgePlatform(QObject *parent);

    virtual void appConnect(ITransportClient *client, const QString &appName) override;
    virtual void appReply(ITransportClient *client, const QByteArray &cmd) override;

    void removeClient(ITransportClient *client) override;

private:
    void execute(ITransportClient *client, const QString &methodName, const QVariantList &paramsArg);

private slots:
    virtual void initializeCommand(ITransportClient *client, const QString &appName) override;
    virtual void appConnectCommand(ITransportClient *client) override;
    virtual void appDisconnectCommand(ITransportClient *client, bool autoLaunch) override;
    virtual void startActivityCommand(ITransportClient *client, const QString &appName, const QVariantList &params) override;
    virtual void installAppCommand(ITransportClient *client, const QString &appPath) override;
    virtual void activateAppCommand(ITransportClient *client, const QString &appId) override;
    virtual void terminateAppCommand(ITransportClient *client, const QString &appId) override;
    virtual void removeAppCommand(ITransportClient *client, const QString &appName) override;
    virtual void isAppInstalledCommand(ITransportClient *client, const QString &rpmName) override;
    virtual void queryAppStateCommand(ITransportClient *client, const QString &appName) override;
    virtual void pushFileCommand(ITransportClient *client, const QString &path, const QString &data) override;
    virtual void pullFileCommand(ITransportClient *client, const QString &path) override;
    virtual void lockCommand(ITransportClient *client, double seconds) override;
    virtual void unlockCommand(ITransportClient *client) override;
    virtual void isLockedCommand(ITransportClient *client) override;
    virtual void launchAppCommand(ITransportClient *client) override;
    virtual void closeAppCommand(ITransportClient *client) override;
    virtual void getCurrentContextCommand(ITransportClient *client) override;
    virtual void getDeviceTimeCommand(ITransportClient *client, const QString &dateFormat) override;
    virtual void setNetworkConnectionCommand(ITransportClient *client, double connectionType) override;
    virtual void getNetworkConnectionCommand(ITransportClient *client) override;
    virtual void resetCommand(ITransportClient *client) override;
    virtual void mobileShakeCommand(ITransportClient *client) override;
    virtual void getSettingsCommand(ITransportClient *client) override;
    virtual void getContextsCommand(ITransportClient *client) override;
    virtual void getCurrentPackageCommand(ITransportClient *client) override;
    virtual void toggleLocationServicesCommand(ITransportClient *client) override;
    virtual void openNotificationsCommand(ITransportClient *client) override;
    virtual void getGeoLocationCommand(ITransportClient *client) override;
    virtual void getLogTypesCommand(ITransportClient *client) override;
    virtual void getLogCommand(ITransportClient *client, const QString &type) override;
    virtual void setGeoLocationCommand(ITransportClient *client, const QVariant &location) override;
    virtual void startRecordingScreenCommand(ITransportClient *client, const QVariant &arguments) override;
    virtual void stopRecordingScreenCommand(ITransportClient *client, const QVariant &arguments) override;
    virtual void executeCommand(ITransportClient *client, const QString &command, const QVariant &paramsArg) override;
    virtual void executeAsyncCommand(ITransportClient *client, const QString &command, const QVariant &paramsArg) override;

// GenericBridgePlatform slots
    void executeCommand_system_shell(ITransportClient *client, const QVariant &executableArg, const QVariant &paramsArg);

    void forwardToApp(ITransportClient *client, const QByteArray &data);
    void forwardToApp(ITransportClient *client, const QString &appName, const QByteArray &data);
    void forwardToApp(ITransportClient *client, const QString &action, const QVariant &params);
    QByteArray sendToAppSocket(const QString &appName, const QByteArray &data);

protected:
    virtual bool lauchAppPlatform(ITransportClient *client) = 0;
    virtual bool lauchAppStandalone(const QString &appName, const QStringList &arguments = {}) = 0;
    void socketReply(ITransportClient *client, const QVariant &value, int status = 0);
    QByteArray actionData(const QString &action, const QVariant &params);

    QHash<ITransportClient*, QString> m_socketAppName;
    QHash<QString, ITransportClient*> m_applicationSocket;
    QHash<ITransportClient*, QString> m_clientFullPath;
    QEventLoop *m_connectLoop;

    QABridge *m_bridge = nullptr;
};

