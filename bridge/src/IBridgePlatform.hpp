// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once
#include <QObject>
#include <QVariant>

class ITransportClient;
class IBridgePlatform : public QObject
{
    Q_OBJECT
public:
    explicit IBridgePlatform(QObject* parent) : QObject(parent) {}

    enum NetworkConnection {
        NetworkConnectionNone = 0,
        NetworkConnectionAirplane = 1,
        NetworkConnectionWifi = 2,
        NetworkConnectionData = 4,
        NetworkConnectionAll = 6
    };
    Q_ENUM(NetworkConnection)

    virtual void appConnect(ITransportClient *client, const QString &appName) = 0;
    virtual void appReply(ITransportClient *client, const QByteArray &cmd) = 0;

    virtual void removeClient(ITransportClient *client) = 0;

signals:
    void applicationReply(ITransportClient *client, const QString &appName, const QByteArray &data);

private slots:
    virtual void initializeCommand(ITransportClient *client, const QString &appName) = 0;
    virtual void appConnectCommand(ITransportClient *client) = 0;
    virtual void appDisconnectCommand(ITransportClient *client, bool autoLaunch) = 0;
    virtual void startActivityCommand(ITransportClient *client, const QString &appName, const QVariantList &params) = 0;
    virtual void installAppCommand(ITransportClient *client, const QString &appPath) = 0;
    virtual void activateAppCommand(ITransportClient *client, const QString &appId) = 0;
    virtual void terminateAppCommand(ITransportClient *client, const QString &appId) = 0;
    virtual void removeAppCommand(ITransportClient *client, const QString &appName) = 0;
    virtual void isAppInstalledCommand(ITransportClient *client, const QString &rpmName) = 0;
    virtual void queryAppStateCommand(ITransportClient *client, const QString &appName) = 0;
    virtual void pushFileCommand(ITransportClient *client, const QString &path, const QString &data) = 0;
    virtual void pullFileCommand(ITransportClient *client, const QString &path) = 0;
    virtual void lockCommand(ITransportClient *client, double seconds) = 0;
    virtual void unlockCommand(ITransportClient *client) = 0;
    virtual void isLockedCommand(ITransportClient *client) = 0;
    virtual void launchAppCommand(ITransportClient *client) = 0;
    virtual void closeAppCommand(ITransportClient *client) = 0;
    virtual void getCurrentContextCommand(ITransportClient *client) = 0;
    virtual void getDeviceTimeCommand(ITransportClient *client, const QString &dateFormat = QString()) = 0;
    virtual void setNetworkConnectionCommand(ITransportClient *client, double connectionType) = 0;
    virtual void getNetworkConnectionCommand(ITransportClient *client) = 0;
    virtual void resetCommand(ITransportClient *client) = 0;
    virtual void mobileShakeCommand(ITransportClient *client) = 0;
    virtual void getSettingsCommand(ITransportClient *client) = 0;
    virtual void getContextsCommand(ITransportClient *client) = 0;
    virtual void getCurrentPackageCommand(ITransportClient *client) = 0;
    virtual void toggleLocationServicesCommand(ITransportClient *client) = 0;
    virtual void openNotificationsCommand(ITransportClient *client) = 0;
    virtual void getGeoLocationCommand(ITransportClient *client) = 0;
    virtual void getLogTypesCommand(ITransportClient *client) = 0;
    virtual void getLogCommand(ITransportClient *client, const QString &type) = 0;
    virtual void setGeoLocationCommand(ITransportClient *client, const QVariant &location) = 0;
    virtual void startRecordingScreenCommand(ITransportClient *client, const QVariant &arguments) = 0;
    virtual void stopRecordingScreenCommand(ITransportClient *client, const QVariant &arguments) = 0;
    virtual void executeCommand(ITransportClient *client, const QString &command, const QVariant &paramsArg) = 0;
    virtual void executeAsyncCommand(ITransportClient *client, const QString &command, const QVariant &paramsArg) = 0;
};
