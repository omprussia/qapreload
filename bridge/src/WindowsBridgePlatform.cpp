// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "WindowsBridgePlatform.hpp"
#include "WinInjector.hpp"

#include <winsock.h>

#include <QDebug>
#include <QProcess>

WindowsBridgePlatform::WindowsBridgePlatform(QObject *parent)
    : GenericBridgePlatform(parent)
{

}

bool WindowsBridgePlatform::lauchAppPlatform(ITransportClient *socket)
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

bool WindowsBridgePlatform::lauchAppStandalone(const QString &appName, const QStringList &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << appName << arguments;

    QProcess process;
    process.setProgram(appName);
    process.setArguments(arguments);
    qint64 pid = 0;
    const bool ret = process.startDetached(&pid);
    if (ret) {
        Injector::injectDll(pid, "qapreloadhook.dll");
    }
    return ret;
}
