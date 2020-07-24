// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once
#include <QObject>
#include <QVariant>

class QTcpSocket;
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

    virtual void appConnect(QTcpSocket *socket, const QString &appName) = 0;
    virtual void appReply(QTcpSocket *socket, const QByteArray &cmd) = 0;

    virtual void removeClient(QTcpSocket *socket) = 0;

signals:
    void applicationReply(QTcpSocket *socket, const QString &appName, const QByteArray &data);

private slots:
    virtual void initializeCommand(QTcpSocket *socket, const QString &appName) = 0;
    virtual void appConnectCommand(QTcpSocket *socket) = 0;
    virtual void appDisconnectCommand(QTcpSocket *socket, bool autoLaunch) = 0;
    virtual void startActivityCommand(QTcpSocket *socket, const QString &appName, const QVariantList &params) = 0;
    virtual void installAppCommand(QTcpSocket *socket, const QString &appPath) = 0;
    virtual void activateAppCommand(QTcpSocket *socket, const QString &appId) = 0;
    virtual void terminateAppCommand(QTcpSocket *socket, const QString &appId) = 0;
    virtual void removeAppCommand(QTcpSocket *socket, const QString &appName) = 0;
    virtual void isAppInstalledCommand(QTcpSocket *socket, const QString &rpmName) = 0;
    virtual void queryAppStateCommand(QTcpSocket *socket, const QString &appName) = 0;
    virtual void pushFileCommand(QTcpSocket *socket, const QString &path, const QString &data) = 0;
    virtual void pullFileCommand(QTcpSocket *socket, const QString &path) = 0;
    virtual void lockCommand(QTcpSocket *socket, double seconds) = 0;
    virtual void unlockCommand(QTcpSocket *socket) = 0;
    virtual void isLockedCommand(QTcpSocket *socket) = 0;
    virtual void launchAppCommand(QTcpSocket *socket) = 0;
    virtual void closeAppCommand(QTcpSocket *socket) = 0;
    virtual void getCurrentContextCommand(QTcpSocket *socket) = 0;
    virtual void getDeviceTimeCommand(QTcpSocket *socket, const QString &dateFormat = QString()) = 0;
    virtual void setNetworkConnectionCommand(QTcpSocket *socket, double connectionType) = 0;
    virtual void getNetworkConnectionCommand(QTcpSocket *socket) = 0;    
    virtual void resetCommand(QTcpSocket *socket) = 0;
    virtual void mobileShakeCommand(QTcpSocket *socket) = 0;
    virtual void getSettingsCommand(QTcpSocket *socket) = 0;
    virtual void getContextsCommand(QTcpSocket *socket) = 0;
    virtual void getCurrentPackageCommand(QTcpSocket *socket) = 0;
    virtual void toggleLocationServicesCommand(QTcpSocket *socket) = 0;
    virtual void openNotificationsCommand(QTcpSocket *socket) = 0;
    virtual void getGeoLocationCommand(QTcpSocket *socket) = 0;
    virtual void getLogTypesCommand(QTcpSocket *socket) = 0;
    virtual void getLogCommand(QTcpSocket *socket, const QString &type) = 0;
    virtual void setGeoLocationCommand(QTcpSocket *socket, const QVariant &location) = 0;
    virtual void startRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments) = 0;
    virtual void stopRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments) = 0;
    virtual void executeCommand(QTcpSocket *socket, const QString &command, const QVariant &paramsArg) = 0;
    virtual void executeAsyncCommand(QTcpSocket *socket, const QString &command, const QVariant &paramsArg) = 0;
};
