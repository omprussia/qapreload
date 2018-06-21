#include "QAEngine.hpp"
#include "QAService.hpp"
#include "qaservice_adaptor.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <QDebug>

#define NAME(x) #x

static QAService *s_instance = nullptr;

bool QAService::initialize(const QString &serviceName)
{
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(serviceName)) {
        qWarning() << Q_FUNC_INFO << "Service name" << serviceName << "is already taken!";
        return false;
    }

    bool success = false;
    success = QDBusConnection::sessionBus().registerObject(QStringLiteral("/ru/omprussia/qaservice"), this);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register object!";
        return success;
    }

    m_adaptor = new QAAdaptor(this);

    success = QDBusConnection::sessionBus().registerService(serviceName);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register service!";
    }

    return success;
}

QAService *QAService::instance()
{
    if (!s_instance) {
        s_instance = new QAService(qApp);
    }
    return s_instance;
}

QAService::QAService(QObject *parent)
    : QObject(parent)
{

}

QString QAService::dumpTree()
{
    qDebug() << Q_FUNC_INFO;

    setDelayedReply(true);
    QMetaObject::invokeMethod(this,
                              NAME(doDumpTree),
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

void QAService::doDumpTree(const QDBusMessage &message)
{
    QString result = QStringLiteral("Hello!");

    sendMessageReply(message, result);


}

QString QAService::findObjectByProperty(const QString &parentObject, const QString &property, const QString &value)
{
    qDebug() << Q_FUNC_INFO << property << value;

    setDelayedReply(true);
    QMetaObject::invokeMethod(this,
                              NAME(doFindObjectByProperty),
                              Qt::QueuedConnection,
                              Q_ARG(QString, parentObject),
                              Q_ARG(QString, property),
                              Q_ARG(QString, value),
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

void QAService::doFindObjectByProperty(const QString &parentObject, const QString &property, const QString &value, const QDBusMessage &message)
{
    QString result = QStringLiteral("object1");

    sendMessageReply(message, result);
}

bool QAService::sendMouseEvent(const QString &object, const QVariantMap &event)
{
    qDebug() << Q_FUNC_INFO << object << event;

    setDelayedReply(true);
    QMetaObject::invokeMethod(this,
                              NAME(doSendMouseEvent),
                              Qt::QueuedConnection,
                              Q_ARG(QString, object),
                              Q_ARG(QVariantMap, event),
                              Q_ARG(QDBusMessage, message()));
    return false;
}

void QAService::doSendMouseEvent(const QString &object, const QVariantMap &event, const QDBusMessage &message)
{
    bool result = true;

    sendMessageReply(message, result);
}

void QAService::sendMessageReply(const QDBusMessage &message, const QVariant &result)
{
    QDBusMessage replyMessage = message.createReply(result);
    QDBusConnection::sessionBus().send(replyMessage);
}
