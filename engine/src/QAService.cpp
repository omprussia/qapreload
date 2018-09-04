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
static QString s_processName;

QByteArray funcName(const char *line)
{
    QByteArray l = QByteArray::fromRawData(line, strlen(line));
    const int to = l.indexOf('(');
    const int from = l.lastIndexOf("::", to) + 2;
    const QByteArray name = l.mid(from, to - from);
    return name;
}

void QAService::initialize()
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
    emit m_adaptor->engineLoaded(QAEngine::isLoaded());

    QDBusMessage applicationReady = QDBusMessage::createMethodCall(
                QStringLiteral("ru.omprussia.qatestrunner"),
                QStringLiteral("/ru/omprussia/qatestrunner"),
                QStringLiteral("ru.omprussia.qatestrunner"),
                QStringLiteral("ApplicationReady"));
    applicationReady.setArguments({s_processName});
    QDBusConnection::sessionBus().call(applicationReady, QDBus::NoBlock);
}

QAService *QAService::instance()
{
    if (!s_instance) {
        s_instance = new QAService(qApp);
    }
    return s_instance;
}

QString QAService::processName()
{
    return s_processName;
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
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, posx),
                              Q_ARG(int, posy),
                              Q_ARG(QDBusMessage, message()));
}

void QAService::pressAndHold(int posx, int posy)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, posx),
                              Q_ARG(int, posy),
                              Q_ARG(QDBusMessage, message()));
}

void QAService::mouseMove(int startx, int starty, int stopx, int stopy)
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

void QAService::pressKeys(const QString &keys)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, keys),
                              Q_ARG(QDBusMessage, message()));
}

void QAService::clearFocus()
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection);
}

QString QAService::executeInPage(const QString &jsCode)
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

QString QAService::executeInWindow(const QString &jsCode)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, jsCode),
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

QString QAService::loadSailfishTest(const QString &fileName)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(QString, fileName),
                              Q_ARG(QDBusMessage, message()));
    return QString();
}

void QAService::clearComponentCache()
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection);
}

void QAService::setEventFilterEnabled(bool enable)
{
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(bool, enable),
                              Q_ARG(QDBusMessage, message()));
}

void QAService::quit()
{
    emit m_adaptor->engineLoaded(false);
    qApp->quit();
}

void QAService::pressEnter(int count)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, count),
                              Q_ARG(QDBusMessage, message()));
}

void QAService::pressBackspace(int count)
{
    setDelayedReply(true);
    QMetaObject::invokeMethod(QAEngine::instance(),
                              METHOD_NAME_HERE,
                              Qt::QueuedConnection,
                              Q_ARG(int, count),
                              Q_ARG(QDBusMessage, message()));
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
