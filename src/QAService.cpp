#include "QAEngine.hpp"
#include "QAService.hpp"
#include "qaservice_adaptor.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <QDebug>

#define NAME(x) #x

#define METHOD_NAME_HERE funcName(Q_FUNC_INFO)

static QAService *s_instance = nullptr;

const char *funcName(const char *line)
{
    QByteArray l = QByteArray::fromRawData(line, strlen(line));
    const int to = l.indexOf('(');
    const int from = l.lastIndexOf("::", to) + 2;
    return l.mid(from, to - from).constData();
}

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
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

QString QAService::dumpCurrentPage()
{
    qDebug() << Q_FUNC_INFO;

    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

QStringList QAService::findObjectsByProperty(const QString &parentObject, const QString &property, const QString &value)
{
    qDebug() << Q_FUNC_INFO << property << value;

    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, parentObject),
                              Q_ARG(QString, property),
                              Q_ARG(QString, value),
                              Q_ARG(QDBusMessage, message()));
    return QStringList();
}

QStringList QAService::findObjectsByClassname(const QString &parentObject, const QString &className)
{
    qDebug() << Q_FUNC_INFO << className;

    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, parentObject),
                              Q_ARG(QString, className),
                              Q_ARG(QDBusMessage, message()));
    return QStringList();
}

bool QAService::clickPoint(int posx, int posy)
{
    qDebug() << Q_FUNC_INFO << posx << posy;

    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, posx),
                              Q_ARG(int, posy),
                              Q_ARG(QDBusMessage, message()));
    return false;
}

bool QAService::clickObject(const QString &object)
{
    qDebug() << Q_FUNC_INFO << object;

    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, object),
                              Q_ARG(QDBusMessage, message()));
    return false;
}

bool QAService::mouseSwipe(int startx, int starty, int stopx, int stopy)
{
    qDebug() << Q_FUNC_INFO << startx << starty << stopx << stopy;

    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, startx),
                              Q_ARG(int, starty),
                              Q_ARG(int, stopx),
                              Q_ARG(int, stopy),
                              Q_ARG(QDBusMessage, message()));
    return false;
}

void QAService::sendMessageReply(const QDBusMessage &message, const QVariant &result)
{
    QDBusMessage replyMessage = message.createReply(result);
    QDBusConnection::sessionBus().send(replyMessage);
}
