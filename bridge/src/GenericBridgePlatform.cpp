#include "GenericBridgePlatform.hpp"
#include "QABridge.hpp"

#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QProcess>
#include <QTcpSocket>
#include <QTimer>
#include <QVariantList>

#include <QDebug>

GenericBridgePlatform::GenericBridgePlatform(QObject *parent)
    : IBridgePlatform(parent)
    , m_connectLoop(new QEventLoop(this))
{
    qDebug()
        << Q_FUNC_INFO;
}

void GenericBridgePlatform::appConnect(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;

    if (m_socketAppName.contains(socket)) {
        m_socketAppName.remove(socket);
    }

    if (m_applicationSocket.value(appName) == nullptr && m_connectLoop->isRunning()) {
        m_connectLoop->quit();
    }

    m_applicationSocket.insert(appName, socket);
}

void GenericBridgePlatform::appReply(QTcpSocket *socket, const QByteArray &cmd)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << cmd;

    const QString appName = m_applicationSocket.key(socket);
    if (appName.isEmpty()) {
        qWarning()
            << Q_FUNC_INFO
            << "No app for" << socket;
        return;
    }

    emit applicationReply(socket, appName, cmd);
}

void GenericBridgePlatform::removeClient(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    if (m_socketAppName.contains(socket)) {
        qDebug()
            << Q_FUNC_INFO
            << "removing client socket:" << m_socketAppName.take(socket);
    }
    QString appName = m_applicationSocket.key(socket);
    if (!appName.isEmpty()) {
        qDebug()
            << Q_FUNC_INFO
            << "removing application socket:" << appName << m_applicationSocket.take(appName);
    }
}

void GenericBridgePlatform::execute(QTcpSocket *socket, const QString &methodName, const QVariantList &params)
{
    bool handled = false;
    bool success = QABridge::metaInvoke(socket, this, methodName, params, &handled);

    if (!handled || !success) {
        qWarning()
            << Q_FUNC_INFO
            << methodName << "not handled!";
        socketReply(socket, QString());
    }
}

void GenericBridgePlatform::initializeCommand(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;

    QString name = appName;
    if (QFileInfo::exists(appName)) {
        name = QFileInfo(appName).baseName();
        qDebug()
            << Q_FUNC_INFO
            << socket
            << "Got app name:" << name
            << "Got full path:" << appName;
        m_clientFullPath.insert(socket, appName);
    }

    if (m_socketAppName.contains(socket)) {
        qWarning()
            << Q_FUNC_INFO
            << "Socket already known:" << m_socketAppName.value(socket);
    }
    m_socketAppName.insert(socket, name);
    if (appName == QLatin1String("headless")) {
        return;
    }
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

    if (m_applicationSocket.value(appName) != nullptr) {
        QTimer maxTimer;
        connect(&maxTimer, &QTimer::timeout, m_connectLoop, &QEventLoop::quit);
        maxTimer.start(30000);
        qDebug()
            << Q_FUNC_INFO
            << "Starting eventloop connect";
        m_connectLoop->exec();
        qDebug()
            << Q_FUNC_INFO
            << "Exiting eventloop connect";
        maxTimer.stop();
    }

    socketReply(socket, QString());
}

void GenericBridgePlatform::appDisconnectCommand(QTcpSocket *socket, bool autoLaunch)
{
    const QString appName = m_socketAppName.value(socket);
    qDebug()
        << Q_FUNC_INFO
        << socket << autoLaunch << appName;

    if (m_applicationSocket.value(appName) != nullptr) {
        if (autoLaunch) {
            sendToAppSocket(appName, actionData(QStringLiteral("closeApp"), QStringList({appName})));
        }

        m_socketAppName.remove(socket);
        qDebug() << Q_FUNC_INFO << appName;
    }

    socketReply(socket, QString());

    if (socket->isOpen()) {
        socket->close();
    }

}

void GenericBridgePlatform::startActivityCommand(QTcpSocket *socket, const QString &appName, const QVariantList &params)
{
    qDebug()
        << Q_FUNC_INFO
        << appName << params;

    QStringList args;
    for (const QVariant &varg : params) {
        args.append(varg.toString());
    }
    const bool success = lauchAppStandalone(appName, args);

    if (success) {
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }

}

void GenericBridgePlatform::installAppCommand(QTcpSocket *socket, const QString &appPath)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appPath;

    socketReply(socket, QString());
}

void GenericBridgePlatform::activateAppCommand(QTcpSocket *socket, const QString &appId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appId;

    const QString appName = m_socketAppName.value(socket);
    qDebug() << Q_FUNC_INFO << appName << appId;
    if (m_applicationSocket.value(appName, nullptr) == nullptr) {
        lauchAppPlatform(socket);
    } else {
        forwardToApp(socket, QStringLiteral("activateApp"), QStringList({appName}));
    }

    socketReply(socket, QString());

}

void GenericBridgePlatform::terminateAppCommand(QTcpSocket *socket, const QString &appId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appId;

    const QString appName = m_socketAppName.value(socket);
    if (m_applicationSocket.value(appName) != nullptr) {
        forwardToApp(socket, QStringLiteral("closeApp"), QStringList({appName}));
        m_applicationSocket.insert(appName, nullptr);
    } else {
        qWarning()
            << Q_FUNC_INFO
            << "App" << appName << "is not active";
        socketReply(socket, false, 1);
    }
}

void GenericBridgePlatform::removeAppCommand(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;

    socketReply(socket, QString());
}

void GenericBridgePlatform::isAppInstalledCommand(QTcpSocket *socket, const QString &rpmName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << rpmName;

    socketReply(socket, QString());
}

void GenericBridgePlatform::queryAppStateCommand(QTcpSocket *socket, const QString &appName)
{
    if (!m_applicationSocket.contains(appName)) {
        socketReply(socket, QStringLiteral("NOT_RUNNING"));
    } else if (m_applicationSocket.value(appName, nullptr) == nullptr) {
        socketReply(socket, QStringLiteral("CLOSING"));
    } else {
        forwardToApp(socket, QStringLiteral("queryAppState"), QStringList({appName}));
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

void GenericBridgePlatform::lockCommand(QTcpSocket *socket, double seconds)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << seconds;

    socketReply(socket, QString());
}

void GenericBridgePlatform::unlockCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    socketReply(socket, QString());
}

void GenericBridgePlatform::isLockedCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    socketReply(socket, QString());
}

void GenericBridgePlatform::launchAppCommand(QTcpSocket *socket)
{
    const QString appName = m_socketAppName.value(socket);
    qDebug()
        << Q_FUNC_INFO
        << socket << appName << m_applicationSocket.value(appName);
    if (m_applicationSocket.value(appName, nullptr) != nullptr) {
        forwardToApp(socket, QStringLiteral("activateApp"), QStringList({appName}));
    } else {
        m_applicationSocket.insert(appName, nullptr);
        qDebug()
            << Q_FUNC_INFO
            << appName;
        lauchAppPlatform(socket);

        QTimer maxTimer;
        connect(&maxTimer, &QTimer::timeout, m_connectLoop, &QEventLoop::quit);
        maxTimer.start(30000);
        qDebug()
            << Q_FUNC_INFO
            << "Starting eventloop connect";
        m_connectLoop->exec();
        qDebug()
            << Q_FUNC_INFO
            << "Exiting eventloop connect";
        maxTimer.stop();
    }

    socketReply(socket, QString());
}

void GenericBridgePlatform::closeAppCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    const QString appName = m_socketAppName.value(socket);
    if (m_applicationSocket.value(appName) != nullptr) {
        QByteArray appReplyData = sendToAppSocket(appName, actionData(QStringLiteral("closeApp"), QStringList({appName})));
        qDebug() << Q_FUNC_INFO << appReplyData;

        QEventLoop loop;
        QTimer timer;
        int counter = 0;
        connect(&timer, &QTimer::timeout, [this, &loop, &counter, appName]() {
            if (!m_applicationSocket.contains(appName)) {
                loop.quit();
            } else {
                counter++;
                if (counter > 10) {
                    loop.quit();
                }
            }
        });
        timer.start(500);
        loop.exec();
        socketReply(socket, QString());
    } else {
        qWarning()
            << Q_FUNC_INFO
            << "App" << appName << "is not active";
        socketReply(socket, QString(), 1);
    }
}

void GenericBridgePlatform::getCurrentContextCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    socketReply(socket, QString());
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

void GenericBridgePlatform::setNetworkConnectionCommand(QTcpSocket *socket, double connectionType)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << connectionType;

    socketReply(socket, QString());
}

void GenericBridgePlatform::getNetworkConnectionCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    socketReply(socket, NetworkConnectionWifi);
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

    const QString fixCommand = QString(command).replace(QChar(':'), QChar('_'));
    const QString methodName = QStringLiteral("executeCommand_%1").arg(fixCommand);
    execute(socket, methodName, paramsArg.toList());
}

void GenericBridgePlatform::executeAsyncCommand(QTcpSocket *socket, const QString &command, const QVariant &paramsArg)
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

    const QString fixCommand = QString(command).replace(QChar(':'), QChar('_'));
    const QString methodName = QStringLiteral("executeCommand_%1").arg(fixCommand);
    execute(socket, methodName, paramsArg.toList());
}

void GenericBridgePlatform::executeCommand_shell(QTcpSocket *socket, const QVariant &executableArg, const QVariant &paramsArg)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << executableArg << paramsArg;

    const QString executable = executableArg.toString();
    const QStringList params = paramsArg.toStringList();

    QProcess p;
    p.start(executable, params);
    p.waitForFinished();


    const QString stdOut = QString::fromUtf8(p.readAllStandardOutput());
    const QString stdErr = QString::fromUtf8(p.readAllStandardError());
    qDebug()
        << Q_FUNC_INFO
        << "stdout:" << stdOut;
    qDebug()
        << Q_FUNC_INFO
        << "stderr:" << stdErr;
    socketReply(socket, stdOut);
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

    qDebug()
        << Q_FUNC_INFO
        << socket << appName << data;

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
    qDebug()
        << Q_FUNC_INFO
        << appName << data.length();

    QJsonObject reply = {
        {QStringLiteral("status"), 1},
        {QStringLiteral("value"), QString()}
    };
    QByteArray appReplyData = QJsonDocument(reply).toJson(QJsonDocument::Compact);
    bool haveData = false;

    QTcpSocket *socket = m_applicationSocket.value(appName, nullptr);
    if (!socket) {
        qDebug()
            << Q_FUNC_INFO
            << "No app socket for" << appName;
    }
    qDebug()
        << Q_FUNC_INFO
        << socket;

    if (!socket || !socket->isOpen()) {
        qWarning()
            << Q_FUNC_INFO
            << "Can't connect to app socket:" << socket;
        return appReplyData;
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(30000);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    QMetaObject::Connection readyReadConnection = connect(
        this, &GenericBridgePlatform::applicationReply,
        [&appReplyData, &loop, &timer, &haveData]
        (QTcpSocket *appSocket, const QString &appName, const QByteArray &data)
    {
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

    socket->write(data);
    socket->waitForBytesWritten();

    if (socket->state() == QAbstractSocket::ConnectedState) {
        timer.start();
        loop.exec();
    }

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
