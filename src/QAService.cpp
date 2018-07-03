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

bool QAService::initialize(bool loaded)
{
    QString processName = qApp->applicationFilePath().section(QLatin1Char('/'), -1);

    if (processName == QStringLiteral("booster-silica-qt5")) {
        processName = qApp->applicationName();
    }

    int serviceSuffix = 0;
    QString finalServiceName = QStringLiteral("ru.omprussia.qaservice.%1").arg(processName);

    while (QDBusConnection::sessionBus().interface()->isServiceRegistered(finalServiceName)) {
        finalServiceName = QStringLiteral("ru.omprussia.qaservice.%1_xx_%2").arg(processName).arg(++serviceSuffix);
    }

    bool success = false;
    success = QDBusConnection::sessionBus().registerObject(QStringLiteral("/ru/omprussia/qaservice"), this);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register object!";
        return success;
    }

    success = QDBusConnection::sessionBus().registerService(finalServiceName);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register service!";
    }

    if (success) {
        m_adaptor = new QAAdaptor(this);
        emit m_adaptor->engineLoaded(loaded);
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
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

QString QAService::dumpCurrentPage()
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

QByteArray QAService::grabWindow()
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QByteArray();
}

QByteArray QAService::grabCurrentPage()
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QByteArray();
}

void QAService::clickPoint(int posx, int posy)
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, posx),
                              Q_ARG(int, posy));
}

void QAService::pressAndHold(int posx, int posy)
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, posx),
                              Q_ARG(int, posy));
}

void QAService::mouseSwipe(int startx, int starty, int stopx, int stopy)
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, startx),
                              Q_ARG(int, starty),
                              Q_ARG(int, stopx),
                              Q_ARG(int, stopy));
}

void QAService::sendMessageReply(const QDBusMessage &message, const QVariant &result)
{
    const QDBusMessage replyMessage = message.createReply(result);
    QDBusConnection::sessionBus().send(replyMessage);
}

void QAService::sendMessageError(const QDBusMessage &message, const QString &errorString)
{
    const QDBusError error(QDBusError::Failed, errorString);
    const QDBusMessage errorMessage = message.createErrorReply(error);
    QDBusConnection::sessionBus().send(errorMessage);
}
