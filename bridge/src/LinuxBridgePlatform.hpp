#pragma once
#include "GenericBridgePlatform.hpp"

class LinuxBridgePlatform : public GenericBridgePlatform
{
    Q_OBJECT
public:
    explicit LinuxBridgePlatform(QObject *parent);

private slots:
    void initializeCommand(QTcpSocket *socket, const QString &appName) override;
    void startActivityCommand(QTcpSocket *socket, const QString &appName, const QVariantList &params) override;
    void installAppCommand(QTcpSocket *socket, const QString &appPath) override;
    void activateAppCommand(QTcpSocket *socket, const QString &appId) override;
    void terminateAppCommand(QTcpSocket *socket, const QString &appId) override;
    void removeAppCommand(QTcpSocket *socket, const QString &appName) override;
    void isAppInstalledCommand(QTcpSocket *socket, const QString &rpmName) override;
    void queryAppStateCommand(QTcpSocket *socket, const QString &appName) override;
    void lockCommand(QTcpSocket *socket, double seconds) override;
    void unlockCommand(QTcpSocket *socket) override;
    void isLockedCommand(QTcpSocket *socket) override;
    void launchAppCommand(QTcpSocket *socket) override;
    void closeAppCommand(QTcpSocket *socket) override;
    void getCurrentContextCommand(QTcpSocket *socket) override;
    void setNetworkConnectionCommand(QTcpSocket *socket, double connectionType) override;
    void getNetworkConnectionCommand(QTcpSocket *socket) override;

private:
    QHash<QTcpSocket*, QString> m_clientFullPath;
};

