#include "GenericBridgePlatform.hpp"

#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTcpSocket>
#include <QTimer>
#include <QVariantList>

#include <QDebug>

GenericBridgePlatform::GenericBridgePlatform(QObject *parent)
    : IBridgePlatform(parent)
{

}

void GenericBridgePlatform::appConnectCommand(QTcpSocket *socket)
{
    const QString appName = m_socketAppName.value(socket);
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;

    if (appName == QLatin1String("headless")) {
        socketReply(socket, QString());
        return;
    }

    // TODO synchronous wait for application

    socketReply(socket, QString());
}

void GenericBridgePlatform::appDisconnectCommand(QTcpSocket *socket, bool autoLaunch)
{
    const QString appName = m_socketAppName.value(socket);
    qDebug()
        << Q_FUNC_INFO
        << socket << appName << autoLaunch;

    socketReply(socket, QString());

    if (socket->isOpen()) {
        socket->close();
    }

}

void GenericBridgePlatform::pushFileCommand(QTcpSocket *socket, const QString &path, const QString &data)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << path << data.size();
    if (data.isEmpty()) {
        qWarning()
            << Q_FUNC_INFO
            << "Received empty file!";
    }

    QFile file(path);
    if (!file.open(QFile::WriteOnly)) {
        qWarning()
            << Q_FUNC_INFO
            << socket << "Error opening file!";
        socketReply(socket, QString(), 1);
        return;
    }
    bool success = file.write(QByteArray::fromBase64(data.toLatin1()));
    if (!success) {
        qWarning()
            << Q_FUNC_INFO
            << socket << "Error saving file!";
        socketReply(socket, QString(), 1);
        return;
    }
    socketReply(socket, QString());
}

void GenericBridgePlatform::pullFileCommand(QTcpSocket *socket, const QString &path)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << path;
    QFile file(path);

    if (!file.open(QFile::ReadOnly)) {
        qWarning()
            << Q_FUNC_INFO
            << socket << "Can't find file" << path << file.errorString();
        socketReply(socket, QString(), 1);
        return;
    }
    const QByteArray data = file.readAll();
    qDebug()
        << Q_FUNC_INFO
        << socket << data.size();

    socketReply(socket, data.toBase64());
}

void GenericBridgePlatform::getDeviceTimeCommand(QTcpSocket *socket, const QString &dateFormat)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << dateFormat;

    QDateTime now = QDateTime::currentDateTime();
    const QString time = dateFormat.isEmpty()
                         ? now.toString(Qt::TextDate)
                         : now.toString(dateFormat);
    socketReply(socket, time);
}

void GenericBridgePlatform::resetCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void GenericBridgePlatform::mobileShakeCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void GenericBridgePlatform::getSettingsCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void GenericBridgePlatform::getContextsCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void GenericBridgePlatform::getCurrentPackageCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void GenericBridgePlatform::toggleLocationServicesCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void GenericBridgePlatform::openNotificationsCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void GenericBridgePlatform::getGeoLocationCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void GenericBridgePlatform::getLogTypesCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QString());
}

void GenericBridgePlatform::getLogCommand(QTcpSocket *socket, const QString &type)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << type;
    socketReply(socket, QString());
}

void GenericBridgePlatform::setGeoLocationCommand(QTcpSocket *socket, const QVariant &location)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << location;
    socketReply(socket, QString());
}

void GenericBridgePlatform::startRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << arguments;
    socketReply(socket, QString());
}

void GenericBridgePlatform::stopRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << arguments;
    socketReply(socket, QString());
}

void GenericBridgePlatform::executeCommand(QTcpSocket *socket, const QString &command, const QVariant &paramsArg)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << command << paramsArg;

    const QStringList commandPair = command.split(QChar(':'));
    if (commandPair.length() != 2) {
        socketReply(socket, QString());
        return;
    }
    if (commandPair.first() != QStringLiteral("system")) {
        forwardToApp(socket, QStringLiteral("execute"), QVariantList({command, paramsArg}));
        return;
    }

    const QVariantList params = paramsArg.toList();
    QGenericArgument arguments[9] = { QGenericArgument() };
    for (int i = 0; i < params.length(); i++) {
        arguments[i] = Q_ARG(QVariant, params[i]);
    }

    bool handled = QMetaObject::invokeMethod(
        this,
        QStringLiteral("executeCommand_%1").arg(commandPair.last()).toLatin1().constData(),
        Qt::DirectConnection,
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

    if (!handled) {
        qWarning() << Q_FUNC_INFO << command << "not handled!";
        socketReply(socket, QString());
    }
}

void GenericBridgePlatform::executeAsyncCommand(QTcpSocket *socket, const QString &command, const QVariant &paramsArg)
{
    const QStringList commandPair = command.split(QChar(':'));
    if (commandPair.length() != 2) {
        socketReply(socket, QString());
        return;
    }
    if (commandPair.first() != QStringLiteral("system")) {
        forwardToApp(socket, QStringLiteral("executeAsync"), QVariantList({command, paramsArg}));
        return;
    }

    socketReply(socket, QString());
}

void GenericBridgePlatform::socketReply(QTcpSocket *socket, const QVariant &value, int status)
{
    QJsonObject reply;
    reply.insert(QStringLiteral("status"), status);
    reply.insert(QStringLiteral("value"), QJsonValue::fromVariant(value));

    qDebug()
        << Q_FUNC_INFO
        << "Reply is:" << reply;

    socket->write(QJsonDocument(reply).toJson(QJsonDocument::Compact));
    socket->flush();
}

void GenericBridgePlatform::forwardToApp(QTcpSocket *socket, const QByteArray &data)
{
    if (!m_socketAppName.contains(socket)) {
        return;
    }

    const QString appName = m_socketAppName.value(socket);

    forwardToApp(socket, appName, data);
}

void GenericBridgePlatform::forwardToApp(QTcpSocket *socket, const QString &appName, const QByteArray &data)
{
    if (!m_applicationSocket.contains(appName)) {
        qWarning()
            << Q_FUNC_INFO
            << "Unknown app:" << appName << socket;
        return;
    }

    QByteArray appReplyData = sendToAppSocket(appName, data);
    qDebug()
        << Q_FUNC_INFO
        << appReplyData;

    socket->write(appReplyData);
    qWarning()
        << Q_FUNC_INFO
        << "Writing to appium socket:"
        << socket->waitForBytesWritten();
}

void GenericBridgePlatform::forwardToApp(QTcpSocket *socket, const QString &action, const QVariant &params)
{
    forwardToApp(socket, actionData(action, params));
}

QByteArray GenericBridgePlatform::sendToAppSocket(const QString &appName, const QByteArray &data)
{
    QJsonObject reply = {
        {QStringLiteral("status"), 1},
        {QStringLiteral("value"), QString()}
    };
    QByteArray appReplyData = QJsonDocument(reply).toJson(QJsonDocument::Compact);
    bool haveData = false;

    QTcpSocket *socket = m_applicationSocket.value(appName, nullptr);
    if (!socket) {
        qWarning()
            << Q_FUNC_INFO
            << "No app socket for" << appName;
    }

    if (socket && !socket->isOpen()) {
        qWarning()
            << Q_FUNC_INFO
            << "Can't connect to app socket:" << socket;
        return appReplyData;
    }

    socket->write(data);
    socket->waitForBytesWritten();

    if (socket->state() != QAbstractSocket::ConnectedState) {
        return appReplyData;
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(30000);

    QMetaObject::Connection readyReadConnection = connect(this, &GenericBridgePlatform::applicationReply,
                                                          [&appReplyData, &loop, &timer, &haveData]
                                                          (QTcpSocket *appSocket, const QString &appName, const QByteArray &data) {
        if (!haveData) {
            haveData = true;
            appReplyData.clear();
        }
        qDebug()
            << Q_FUNC_INFO
            << "Received bytes from app:" << appName << appSocket;
        appReplyData.append(data);

        QJsonParseError jsonError;
        QJsonDocument::fromJson(appReplyData, &jsonError);
        if (jsonError.error == QJsonParseError::NoError) {
            loop.quit();
        } else {
            timer.start();
        }

    });

    QMetaObject::Connection stateChangedConnection = connect(socket, &QAbstractSocket::stateChanged, this, [&loop, &timer](QAbstractSocket::SocketState state) {
        if (state != QAbstractSocket::ConnectedState) {
            loop.quit();
            timer.stop();
        }
    });
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();

    disconnect(readyReadConnection);
    disconnect(stateChangedConnection);

    return appReplyData;
}

QByteArray GenericBridgePlatform::actionData(const QString &action, const QVariant &params)
{
    QJsonObject json;
    json.insert(QStringLiteral("cmd"), QJsonValue(QStringLiteral("action")));
    json.insert(QStringLiteral("action"), QJsonValue(action));
    json.insert(QStringLiteral("params"), QJsonValue::fromVariant(params));
    return QJsonDocument(json).toJson(QJsonDocument::Compact);
}
