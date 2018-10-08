#include "QABridge.hpp"

#include <QDebug>
#include <QHash>

#include <QTcpServer>
#include <QTcpSocket>

#include <QCoreApplication>
#include <QDateTime>
#include <QMetaMethod>

#include <QJsonObject>
#include <QJsonDocument>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

#include <systemd/sd-daemon.h>


static QHash<QTcpSocket*, QString> s_appSocket;
static QHash<QString, int> s_appPort;

QABridge::QABridge(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &QABridge::newConnection);

    qRegisterMetaType<QTcpSocket*>();
}

void QABridge::start()
{
    qDebug() << Q_FUNC_INFO;

    if (sd_listen_fds(0) == 1) {
        int fd = SD_LISTEN_FDS_START;
        qDebug() << Q_FUNC_INFO << "Using systemd socket descriptor:" << fd <<
                    m_server->setSocketDescriptor(fd);
    } else if (!m_server->listen(QHostAddress::AnyIPv4, 8888)) {
        qWarning() << Q_FUNC_INFO << m_server->errorString();
        qApp->quit();
    }
}

void QABridge::newConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &QABridge::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &QABridge::removeSocket);
    qDebug() << Q_FUNC_INFO << "New connection from:" << socket->peerAddress() << socket->peerPort();
}

void QABridge::readSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    const QByteArray requestData = socket->readAll();
    qDebug() << Q_FUNC_INFO << socket << requestData;

    QJsonParseError error;
    QJsonDocument json = QJsonDocument::fromJson(requestData, &error);
    if (error.error != QJsonParseError::NoError) {
        return;
    }

    QJsonObject object = json.object();
    if (!object.contains(QStringLiteral("cmd"))) {
        return;
    }

    if (object.value(QStringLiteral("cmd")).toVariant().toString() != QStringLiteral("action")) {
        return;
    }

    if (!object.contains(QStringLiteral("action"))) {
        return;
    }

    const QString action = object.value(QStringLiteral("action")).toVariant().toString();
    QVariantList params = object.value(QStringLiteral("params")).toVariant().toList();
    if (params.length() > 9) {
        qWarning() << Q_FUNC_INFO << "Too many params for" << action;
        params = params.mid(0, 9);
    }

    const QString methodName = QStringLiteral("%1Bootstrap").arg(action);

    QGenericArgument arguments[9] = { QGenericArgument() };
    for (int i = 0; i < params.length(); i++) {
        arguments[i] = Q_ARG(QVariant, params[i]);
    }

    for (int i = metaObject()->methodOffset(); i < metaObject()->methodOffset() + metaObject()->methodCount(); i++) {
        if (metaObject()->method(i).name() == methodName.toLatin1()) {
            QMetaObject::invokeMethod(this,
                                      methodName.toLatin1().constData(),
                                      Qt::QueuedConnection,
                                      Q_ARG(QTcpSocket*, socket),
                                      arguments[0],
                                      arguments[1],
                                      arguments[2],
                                      arguments[3],
                                      arguments[4],
                                      arguments[5],
                                      arguments[6],
                                      arguments[7],
                                      arguments[8]);
            return;
        }
    }

    if (s_appSocket.contains(socket)) {
        forwardToApp(socket, requestData);
    }
}

void QABridge::removeSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    qDebug() << Q_FUNC_INFO << socket;
    if (s_appSocket.contains(socket)) {
        qDebug() << Q_FUNC_INFO << s_appSocket.value(socket);
        s_appPort.remove(s_appSocket.value(socket));
        s_appSocket.remove(socket);
    }
}

void QABridge::appConnectBootstrap(QTcpSocket *socket, const QVariant &arguments)
{
    const QString appName = arguments.toString();

    if (s_appSocket.contains(socket)) {
        qWarning() << Q_FUNC_INFO << "Socket already known:" << s_appSocket.value(socket);
    }
    s_appSocket.insert(socket, appName);

    QDBusMessage startAppSocket = QDBusMessage::createMethodCall(
                QStringLiteral("ru.omprussia.qaservice.%1").arg(appName),
                QStringLiteral("/ru/omprussia/qaservice"),
                QStringLiteral("ru.omprussia.qaservice"),
                QStringLiteral("startSocket"));
    QDBusReply<int> reply = QDBusConnection::sessionBus().call(startAppSocket);
    if (reply.value() > 0) {
        s_appPort.insert(appName, reply.value());
    }
    qDebug() << Q_FUNC_INFO << reply.value();

//    socketReply(socket, reply.value());
}

void QABridge::activateAppBootstrap(QTcpSocket *socket, const QVariant &appId, const QVariant &, const QVariant &, const QVariant &)
{
    socketReply(socket, QString());
}

void QABridge::terminateAppBootstrap(QTcpSocket *socket, const QVariant &appId, const QVariant &, const QVariant &, const QVariant &)
{
    socketReply(socket, QString());
}

void QABridge::installAppBootstrap(QTcpSocket *socket, const QVariant &appPath, const QVariant &argument1, const QVariant &sessionId, const QVariant &)
{
    qDebug() << Q_FUNC_INFO << appPath;
    socketReply(socket, QString());
}

void QABridge::removeAppBootstrap(QTcpSocket *socket, const QVariant &appName, const QVariant &, const QVariant &, const QVariant &)
{
    qDebug() << Q_FUNC_INFO << appName;
    socketReply(socket, QString());
}

void QABridge::isAppInstalledBootstrap(QTcpSocket *socket, const QVariant &appName)
{
    qDebug() << Q_FUNC_INFO << appName;
    socketReply(socket, QString());
}

void QABridge::queryAppStateBootstrap(QTcpSocket *socket, const QVariant &appId)
{
    socketReply(socket, QString());
}

void QABridge::launchAppBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::closeAppBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::resetBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::backgroundBootstrap(QTcpSocket *socket, const QVariant &second)
{
    qDebug() << Q_FUNC_INFO << second;
    socketReply(socket, QString());
}

void QABridge::setNetworkConnectionBootstrap(QTcpSocket *socket, const QVariant &connectionType)
{
    socketReply(socket, QString());
}

void QABridge::getNetworkConnectionBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::availableIMEEnginesBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::activateIMEEngineBootstrap(QTcpSocket *socket, const QVariant &engine)
{
    qDebug() << Q_FUNC_INFO << engine;
    socketReply(socket, QString());
}

void QABridge::getStringsBootstrap(QTcpSocket *socket, const QVariant &language, const QVariant &stringFile)
{
    qDebug() << Q_FUNC_INFO << language << stringFile;
    socketReply(socket, QString());
}

void QABridge::endCoverageBootstrap(QTcpSocket *socket, const QVariant &intent, const QVariant &path)
{
    qDebug() << Q_FUNC_INFO << intent << path;
    socketReply(socket, QString());
}

void QABridge::getClipboardBootstrap(QTcpSocket *socket, const QVariant &contentType)
{
    socketReply(socket, QString());
}

void QABridge::getActiveIMEEngineBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::deactivateIMEEngineBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::mobileShakeBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::getSettingsBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::getDeviceTimeBootstrap(QTcpSocket *socket, const QVariant &format)
{
    socketReply(socket, QDateTime::currentDateTime().toString());
}

void QABridge::startRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments)
{
    socketReply(socket, QString());
}

void QABridge::stopRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments)
{
    socketReply(socket, QString());
}

void QABridge::getCurrentContextBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString("NATIVE_APP"));
}

void QABridge::getContextsBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::getCurrentPackageBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::toggleLocationServicesBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::openNotificationsBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::getGeoLocationBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::getLogTypesBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QStringList());
}

void QABridge::getLogBootstrap(QTcpSocket *socket, const QVariant &typeArg)
{
    socketReply(socket, QStringList());
}

void QABridge::setGeoLocationBootstrap(QTcpSocket *socket, const QVariant &location)
{
    socketReply(socket, QString());
}

void QABridge::lockBootstrap(QTcpSocket *socket, const QVariant &seconds)
{
    qDebug() << Q_FUNC_INFO << seconds;
    socketReply(socket, QString());
}

void QABridge::executeBootstrap(QTcpSocket *socket, const QVariant &scriptArg, const QVariant &paramsArg)
{
    socketReply(socket, QString());
}

void QABridge::executeAsyncBootstrap(QTcpSocket *socket, const QVariant &scriptArg, const QVariant &paramsArg)
{
    socketReply(socket, QString());
}

void QABridge::pushFileBootstrap(QTcpSocket *socket, const QVariant &pathArg, const QVariant &dataArg)
{
    socketReply(socket, QString());
}

void QABridge::pullFileBootstrap(QTcpSocket *socket, const QVariant &pathArg)
{
    socketReply(socket, QString());
}

void QABridge::pullFolderBootstrap(QTcpSocket *socket, const QVariant &pathArg)
{
    socketReply(socket, QString());
}

void QABridge::touchIdBootstrap(QTcpSocket *socket, const QVariant &matchArg)
{
    socketReply(socket, QString());
}

void QABridge::toggleEnrollTouchIdBootstrap(QTcpSocket *socket, const QVariant &)
{
    socketReply(socket, true);

}

void QABridge::implicitWaitBootstrap(QTcpSocket *socket, const QVariant &msecondArg)
{
    socketReply(socket, QString());
}

void QABridge::asyncScriptTimeoutBootstrap(QTcpSocket *socket, const QVariant &msecondArg)
{
    socketReply(socket, QString());
}

void QABridge::timeoutsBootstrap(QTcpSocket *socket, const QVariant &, const QVariant &, const QVariant &, const QVariant &msecondArg, const QVariant &)
{
    socketReply(socket, QString());
}

void QABridge::compareImagesBootstrap(QTcpSocket *socket, const QVariant &matchFeatures, const QVariant &firstImage, const QVariant &secondImage, const QVariant &, const QVariant &, const QVariant &)
{
    socketReply(socket, QString());
}

void QABridge::unlockBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::isLockedBootstrap(QTcpSocket *socket)
{
    socketReply(socket, false);
}

void QABridge::forwardToApp(QTcpSocket *socket, const QByteArray &data)
{
    const QString appName = s_appSocket.value(socket);

    if (!s_appPort.contains(appName)) {
        qWarning() << Q_FUNC_INFO << "Unknown app:" << socket;
        return;
    }

    QTcpSocket appSocket;
    appSocket.connectToHost(QHostAddress(QHostAddress::LocalHost), s_appPort.value(appName));
    if (!appSocket.isOpen()) {
        qWarning() << Q_FUNC_INFO << "Can't connect to app socket:" << s_appPort.value(appName);
    }
    qWarning() << Q_FUNC_INFO << "Writing to app socket:" <<
                  appSocket.write(data) <<
                  appSocket.waitForBytesWritten();

    qDebug() << Q_FUNC_INFO << "Reading from app socket:" <<
    appSocket.waitForReadyRead();
    const QByteArray appReplyData = appSocket.readAll();
    qDebug() << Q_FUNC_INFO << appReplyData;

    appSocket.close();

    socket->write(appReplyData);
    qWarning() << Q_FUNC_INFO << "Writing to appium socket:" <<
    socket->waitForBytesWritten();
}

void QABridge::socketReply(QTcpSocket *socket, const QVariant &value, int status)
{
    QJsonObject reply;
    reply.insert(QStringLiteral("status"), status);
    reply.insert(QStringLiteral("value"), QJsonValue::fromVariant(value));

    qDebug() << Q_FUNC_INFO << "Reply is:" << reply;
    socket->write(QJsonDocument(reply).toJson(QJsonDocument::Compact));
    socket->flush();
}

