#include "LinuxBridgePlatform.hpp"

#include <QDebug>
#include <QFileInfo>

LinuxBridgePlatform::LinuxBridgePlatform(QObject *parent)
    : GenericBridgePlatform(parent)
{

}

void LinuxBridgePlatform::initializeCommand(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;

    QString name = appName;
    if (QFileInfo::exists(appName)) {
        name = QFileInfo(appName).baseName();
        qDebug()
            << Q_FUNC_INFO
            << socket
            << "Got app name:" << name
            << "Got full path:" << appName;
        m_clientFullPath.insert(socket, appName);
    }

    if (m_socketAppName.contains(socket)) {
        qWarning()
            << Q_FUNC_INFO
            << "Socket already known:" << m_socketAppName.value(socket);
    }
    m_socketAppName.insert(socket, name);
    if (appName == QLatin1String("headless")) {
        return;
    }
}

void LinuxBridgePlatform::startActivityCommand(QTcpSocket *socket, const QString &appName, const QVariantList &params)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName << params;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::installAppCommand(QTcpSocket *socket, const QString &appPath)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appPath;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::activateAppCommand(QTcpSocket *socket, const QString &appId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appId;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::terminateAppCommand(QTcpSocket *socket, const QString &appId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appId;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::removeAppCommand(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::isAppInstalledCommand(QTcpSocket *socket, const QString &rpmName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << rpmName;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::queryAppStateCommand(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::lockCommand(QTcpSocket *socket, double seconds)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << seconds;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::unlockCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::isLockedCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::launchAppCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::closeAppCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::getCurrentContextCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::setNetworkConnectionCommand(QTcpSocket *socket, double connectionType)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << connectionType;
    socketReply(socket, QString());
}

void LinuxBridgePlatform::getNetworkConnectionCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}
