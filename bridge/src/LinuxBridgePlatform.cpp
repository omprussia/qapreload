#include "LinuxBridgePlatform.hpp"

#include <QDebug>
#include <QFileInfo>
#include <QProcess>

LinuxBridgePlatform::LinuxBridgePlatform(QObject *parent)
    : GenericBridgePlatform(parent)
{

}

bool LinuxBridgePlatform::lauchAppPlatform(QTcpSocket *socket)
{
    QString appName = m_socketAppName.value(socket);
    if (m_clientFullPath.contains(socket)) {
        appName = m_clientFullPath.value(socket);
    }

    qDebug()
        << Q_FUNC_INFO
        << socket << appName;

    return lauchAppStandalone(appName);
}

bool LinuxBridgePlatform::lauchAppStandalone(const QString &appName, const QStringList &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << appName << arguments;

    QProcess *process = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_PRELOAD", "/usr/lib/libqapreloadhook.so");
    process->setProcessEnvironment(env);
    process->setProgram(appName);
    process->setArguments(arguments);
    process->start();

    return process->state() != QProcess::NotRunning;
}
