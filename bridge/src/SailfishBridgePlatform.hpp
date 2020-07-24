// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once
#include "LinuxBridgePlatform.hpp"

class QAScreenRecorder;
class SailfishBridgePlatform : public LinuxBridgePlatform
{
    Q_OBJECT
public:
    explicit SailfishBridgePlatform(QObject *parent);

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

private:
    QAScreenRecorder *m_screenrecorder = nullptr;

protected:
    bool lauchAppStandalone(const QString &appName, const QStringList &arguments) override;
};

