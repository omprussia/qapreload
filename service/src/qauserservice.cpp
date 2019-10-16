#include "qauserservice.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <QCoreApplication>

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
    ContentAction::Action action = ContentAction::Action::launcherAction(QStringLiteral("%1.desktop").arg(appName), arguments);
    action.trigger();
}
