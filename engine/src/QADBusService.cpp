#include "QAEngine.hpp"
#include "QADBusService.hpp"
#include "QASocketService.hpp"
#include "qaservice_adaptor.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <QDebug>

#define NAME(x) #x

#define METHOD_NAME_HERE funcName(Q_FUNC_INFO)

static QADBusService *s_instance = nullptr;
static QString s_processName;

QByteArray funcName(const char *line)
{
    QByteArray l = QByteArray::fromRawData(line, strlen(line));
    const int to = l.indexOf('(');
    const int from = l.lastIndexOf("::", to) + 2;
    const QByteArray name = l.mid(from, to - from);
    return name;
}

void QADBusService::initialize()
{
    if (m_adaptor) {
        return;
    }

    s_processName = qApp->arguments().first().section(QLatin1Char('/'), -1);

    int serviceSuffix = 0;
    QString finalServiceName = QStringLiteral("ru.omprussia.qaservice.%1").arg(s_processName);

    while (QDBusConnection::sessionBus().interface()->isServiceRegistered(finalServiceName)) {
        finalServiceName = QStringLiteral("ru.omprussia.qaservice.%1_xx_%2").arg(s_processName).arg(++serviceSuffix);
    }

    bool success = false;
    success = QDBusConnection::sessionBus().registerObject(QStringLiteral("/ru/omprussia/qaservice"), this);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register object!";
        return;
    }

    success = QDBusConnection::sessionBus().registerService(finalServiceName);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register service!";
    }

    if (!success) {
        return;
    }

    m_adaptor = new QAAdaptor(this);
    emit engineLoaded(QAEngine::isLoaded());
}

QADBusService *QADBusService::instance()
{
    if (!s_instance) {
        s_instance = new QADBusService(qApp);
    }
    return s_instance;
}

QString QADBusService::processName()
{
    return s_processName;
}

QADBusService::QADBusService(QObject *parent)
    : QObject(parent)
{

}

QString QADBusService::dumpTree()
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

QString QADBusService::dumpCurrentPage()
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

QByteArray QADBusService::grabWindow()
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QByteArray();
}

QByteArray QADBusService::grabCurrentPage()
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
    return QByteArray();
}

void QADBusService::clickPoint(int posx, int posy)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, posx),
                              Q_ARG(int, posy),
                              Q_ARG(QDBusMessage, message()));
}

void QADBusService::pressAndHold(int posx, int posy)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, posx),
                              Q_ARG(int, posy),
                              Q_ARG(QDBusMessage, message()));
}

void QADBusService::mouseMove(int startx, int starty, int stopx, int stopy)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, startx),
                              Q_ARG(int, starty),
                              Q_ARG(int, stopx),
                              Q_ARG(int, stopy),
                              Q_ARG(QDBusMessage, message()));
}

void QADBusService::pressKeys(const QString &keys)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, keys),
                              Q_ARG(QDBusMessage, message()));
}

void QADBusService::clearFocus()
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection);
}

QString QADBusService::executeInPage(const QString &jsCode)
{
    qWarning() << Q_FUNC_INFO << jsCode;
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, jsCode),
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

QString QADBusService::executeInWindow(const QString &jsCode)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, jsCode),
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

QString QADBusService::loadSailfishTest(const QString &fileName)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, fileName),
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

void QADBusService::clearComponentCache()
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection);
}

void QADBusService::setEventFilterEnabled(bool enable)
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(bool, enable),
                              Q_ARG(QDBusMessage, message()));
}

void QADBusService::setTouchIndicatorEnabled(bool enable)
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(bool, enable),
                              Q_ARG(QDBusMessage, message()));
}

void QADBusService::hideTouchIndicator()
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QDBusMessage, message()));
}

void QADBusService::startSocket()
{
    qWarning() << Q_FUNC_INFO;
    QASocketService *socket = QASocketService::instance();
    socket->connectToBridge();
}

void QADBusService::quit()
{
    emit m_adaptor->engineLoaded(false);
    qApp->quit();
}

void QADBusService::pressEnter(int count)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, count),
                              Q_ARG(QDBusMessage, message()));
}

void QADBusService::pressBackspace(int count)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, count),
                              Q_ARG(QDBusMessage, message()));
}

void QADBusService::sendMessageReply(const QDBusMessage &message, const QVariant &result)
{
    const QDBusMessage replyMessage = message.createReply(result);
    QDBusConnection::sessionBus().send(replyMessage);
}

void QADBusService::sendMessageError(const QDBusMessage &message, const QString &errorString)
{
    const QDBusError error(QDBusError::Failed, errorString);
    const QDBusMessage errorMessage = message.createErrorReply(error);
    QDBusConnection::sessionBus().send(errorMessage);
}
