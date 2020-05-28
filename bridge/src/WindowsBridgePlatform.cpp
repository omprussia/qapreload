// Copyright (c) 2020 Open Mobile Platform LLС.
#include "WindowsBridgePlatform.hpp"
#include "WinInjector.hpp"

#include <winsock.h>

#include <QDebug>

WindowsBridgePlatform::WindowsBridgePlatform(QObject *parent)
    : GenericBridgePlatform(parent)
{

}

bool WindowsBridgePlatform::lauchAppPlatform(QTcpSocket *socket)
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
        Injector::injectDll(pid, QStringLiteral("qapreloadhook.dll"));
    }
    return ret;
}
