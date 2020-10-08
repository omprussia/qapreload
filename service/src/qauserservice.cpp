// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "qauserservice.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QProcess>

#include <contentaction5/contentaction.h>

QAUserService::QAUserService(QObject *parent) : QObject(parent)
{

}

void QAUserService::start()
{
    const QString serviceName = QStringLiteral("ru.omprussia.qaservice");
    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (sessionBus.interface()->isServiceRegistered(serviceName)) {
        qWarning() << Q_FUNC_INFO << "Service already registered!";
        qApp->quit();
        return;
    }

    bool success = false;
    success = sessionBus.registerObject(QStringLiteral("/ru/omprussia/qaservice"), this, QDBusConnection::ExportScriptableSlots);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register object!";
        qApp->quit();
        return;
    }

    success = sessionBus.registerService(serviceName);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register service!";
        qApp->quit();
    }
}

void QAUserService::launchApp(const QString &appName, const QStringList &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << appName << arguments;

    if (QFileInfo::exists(appName)) {
        launchProcess(appName, arguments);
    } else {
        launcherAction(appName, arguments);
    }
}

void QAUserService::launcherAction(const QString &appName, const QStringList &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << appName << arguments;

    ContentAction::Action action = ContentAction::Action::launcherAction(QStringLiteral("%1.desktop").arg(appName), arguments);
    action.trigger();
}

void QAUserService::launchProcess(const QString &appName, const QStringList &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << appName << arguments;

    QProcess *process = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LD_PRELOAD", INSTALLS_LIBDIR"/libqapreloadhook.so");
    process->setProcessEnvironment(env);
    process->setProgram(appName);
    process->setArguments(arguments);
    process->start();
    connect(process, static_cast<void(QProcess::*)(int)>(&QProcess::finished), [process](int exitCode) {
        qDebug()
            << Q_FUNC_INFO
            << "process" << process->program()
            << "finished with code:" << exitCode;
        process->deleteLater();
    });
}
