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

class QAScreenRecorder;
class SailfishBridgePlatform : public LinuxBridgePlatform
{
    Q_OBJECT
public:
    explicit SailfishBridgePlatform(QObject *parent);
    static pid_t findProcess(const char *appName);

private slots:
    void installAppCommand(QTcpSocket *socket, const QString &appPath) override;
    void removeAppCommand(QTcpSocket *socket, const QString &appName) override;
    void isAppInstalledCommand(QTcpSocket *socket, const QString &rpmName) override;
    void lockCommand(QTcpSocket *socket, double seconds) override;
    void unlockCommand(QTcpSocket *socket) override;
    void isLockedCommand(QTcpSocket *socket) override;
    void setNetworkConnectionCommand(QTcpSocket *socket, double connectionType) override;
    void getNetworkConnectionCommand(QTcpSocket *socket) override;
    void startRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments) override;
    void stopRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments) override;

// SailfishBridgePlatform slots
    void executeCommand_system_unlock(QTcpSocket *socket, const QVariant &executableArg, const QVariant &paramsArg);

// login manager
    void userNew(uint userId, const QDBusObjectPath &userPath);

private:
    QAScreenRecorder *m_screenrecorder = nullptr;

protected:
    bool lauchAppStandalone(const QString &appName, const QStringList &arguments) override;
};

