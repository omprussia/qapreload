// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once
#include "LinuxBridgePlatform.hpp"

#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusObjectPath>

struct LoginUserData {
   uint id = 0;
   QString name;
   QDBusObjectPath path;
};
Q_DECLARE_METATYPE(LoginUserData)

QDBusArgument &operator<<(QDBusArgument &argument, const LoginUserData &data);
const QDBusArgument &operator>>(const QDBusArgument &argument, LoginUserData &data);

class ITransportServer;
class QAScreenRecorder;
class SailfishBridgePlatform : public LinuxBridgePlatform
{
    Q_OBJECT
public:
    explicit SailfishBridgePlatform(QObject *parent);
    static pid_t findProcess(const QString &appName);

private slots:
    void installAppCommand(ITransportClient *socket, const QString &appPath) override;
    void removeAppCommand(ITransportClient *socket, const QString &appName) override;
    void isAppInstalledCommand(ITransportClient *socket, const QString &rpmName) override;
    void lockCommand(ITransportClient *socket, double seconds) override;
    void unlockCommand(ITransportClient *socket) override;
    void isLockedCommand(ITransportClient *socket) override;
    void setNetworkConnectionCommand(ITransportClient *socket, double connectionType) override;
    void getNetworkConnectionCommand(ITransportClient *socket) override;
    void startRecordingScreenCommand(ITransportClient *socket, const QVariant &arguments) override;
    void stopRecordingScreenCommand(ITransportClient *socket, const QVariant &arguments) override;

// SailfishBridgePlatform slots
    void executeCommand_system_unlock(ITransportClient *socket, const QVariant &executableArg, const QVariant &paramsArg);

// login manager
    void userNew(uint userId, const QDBusObjectPath &userPath);

private:
    ITransportServer *m_rpc = nullptr;
    QAScreenRecorder *m_screenrecorder = nullptr;

protected:
    bool lauchAppStandalone(const QString &appName, const QStringList &arguments) override;
};

