#include "QABridge.hpp"
#include "qabridge_adaptor.h"

#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMetaMethod>
#include <QTimer>

#include <QJsonObject>
#include <QJsonDocument>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

#include <contentaction5/contentaction.h>

#include <Transaction>
#include <Daemon>

#include <connman-qt5/networkmanager.h>

#include <systemd/sd-daemon.h>

QABridge::QABridge(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_connectLoop(new QEventLoop(this))
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
        return;
    }

    if (m_adaptor) {
        return;
    }

    if (QDBusConnection::systemBus().interface()->isServiceRegistered(DBUS_SERVICE_NAME)) {
        qWarning() << Q_FUNC_INFO << "Service already registered!";
        return;
    }

    bool success = false;
    success = QDBusConnection::systemBus().registerObject(DBUS_PATH_NAME, this);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register object!";
        return;
    }

    success = QDBusConnection::systemBus().registerService(DBUS_SERVICE_NAME);
    if (!success) {
        qWarning () << Q_FUNC_INFO << "Failed to register service!" << QDBusConnection::systemBus().lastError().message();
        return;
    }

    qDebug() << Q_FUNC_INFO << "Started D-Bus service" << DBUS_SERVICE_NAME;

    m_adaptor = new QABridgeAdaptor(this);
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

    qDebug() << Q_FUNC_INFO << socket << socket->bytesAvailable();
    const QByteArray requestData = socket->readAll();
    qDebug().noquote() << requestData;

    const QList<QByteArray> commands = requestData.split('\n');
    for (const QByteArray &cmd : commands) {
        if (cmd.isEmpty()) {
            continue;
        }
        processCommand(socket, cmd);
    }
}

void QABridge::removeSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    qDebug() << Q_FUNC_INFO << socket;
    if (m_appSocket.contains(socket)) {
        qDebug() << Q_FUNC_INFO << m_appSocket.value(socket);
        m_appPort.remove(m_appSocket.value(socket));
        m_appSocket.remove(socket);
    }
}

void QABridge::initializeBootstrap(QTcpSocket *socket, const QVariant &appPackageArg)
{
    const QString appName = appPackageArg.toString();
    qDebug() << Q_FUNC_INFO << appName;

    if (m_appSocket.contains(socket)) {
        qWarning() << Q_FUNC_INFO << "Socket already known:" << m_appSocket.value(socket);
    }
    m_appSocket.insert(socket, appName);
    m_appPort.insert(appName, 0);
}

void QABridge::appConnectBootstrap(QTcpSocket *socket)
{
    const QString appName = m_appSocket.value(socket);

    connectAppSocket(appName);
    qDebug() << Q_FUNC_INFO << m_appPort.value(appName);

    if (m_appPort.value(appName) == 0) {
        QTimer maxTimer;
        connect(&maxTimer, &QTimer::timeout, m_connectLoop, &QEventLoop::quit);
        maxTimer.start(30000);
        qDebug() << Q_FUNC_INFO << "Starting eventloop connect";
        m_connectLoop->exec();
        qDebug() << Q_FUNC_INFO << "Exiting eventloop connect";
        maxTimer.stop();
    }

    qDebug() << Q_FUNC_INFO << m_appPort.value(appName);
    socketReply(socket, QString());
}

void QABridge::appDisconnectBootstrap(QTcpSocket *socket)
{
    const QString appName = m_appSocket.value(socket);
    m_appPort.remove(appName);
    m_appSocket.remove(socket);
    qDebug() << Q_FUNC_INFO << m_appPort.value(appName);
    socketReply(socket, QString());
}

void QABridge::activateAppBootstrap(QTcpSocket *socket, const QVariant &appIdArg, const QVariant &, const QVariant &, const QVariant &)
{
    const QString appName = m_appSocket.value(socket);
    qDebug() << Q_FUNC_INFO << appName;
    if (m_appPort.value(appName, 0) == 0) {
        QStringList arguments;
        ContentAction::Action action = ContentAction::Action::launcherAction(QStringLiteral("%1.desktop").arg(appName), arguments);
        action.trigger();
    } else {
        QJsonObject json;
        json.insert(QStringLiteral("cmd"), QJsonValue(QStringLiteral("action")));
        json.insert(QStringLiteral("action"), QJsonValue(QStringLiteral("activateApp")));
        json.insert(QStringLiteral("params"), QJsonValue::fromVariant(QStringList({appName})));
        forwardToApp(socket, QJsonDocument(json).toJson(QJsonDocument::Compact));
    }
    socketReply(socket, QString());
}

void QABridge::terminateAppBootstrap(QTcpSocket *socket, const QVariant &appId, const QVariant &, const QVariant &, const QVariant &)
{
    const QString appName = m_appSocket.value(socket);
    qDebug() << Q_FUNC_INFO << appName;
    if (m_appPort.value(appName) != 0) {
        QJsonObject json;
        json.insert(QStringLiteral("cmd"), QJsonValue(QStringLiteral("action")));
        json.insert(QStringLiteral("action"), QJsonValue(QStringLiteral("closeApp")));
        json.insert(QStringLiteral("params"), QJsonValue::fromVariant(QStringList({appName})));
        forwardToApp(socket, QJsonDocument(json).toJson(QJsonDocument::Compact));
        m_appPort.insert(appName, 0);
    } else {
        qWarning() << Q_FUNC_INFO << "App" << appName << "is not active";
        socketReply(socket, false, 1);
    }
}

void QABridge::installAppBootstrap(QTcpSocket *socket, const QVariant &appPathArg, const QVariant &argument1, const QVariant &sessionId, const QVariant &)
{
    const QString appPath = appPathArg.toString();
    qDebug() << Q_FUNC_INFO << appPath;
    QEventLoop loop;
    PackageKit::Transaction *tx = PackageKit::Daemon::installFile(appPath, PackageKit::Transaction::TransactionFlagNone);
    connect(tx, &PackageKit::Transaction::finished, [&loop, socket, this](PackageKit::Transaction::Exit status, uint) {
        qDebug() << Q_FUNC_INFO << status;
        socketReply(socket, QString(), status == PackageKit::Transaction::ExitSuccess ? 0 : 1);

        loop.quit();
    });
    loop.exec();
}

void QABridge::removeAppBootstrap(QTcpSocket *socket, const QVariant &appNameArg, const QVariant &, const QVariant &, const QVariant &)
{
    const QString appName = appNameArg.toString();
    qDebug() << Q_FUNC_INFO << appName;
    QEventLoop loop;
    PackageKit::Transaction *r = PackageKit::Daemon::resolve(appName, PackageKit::Transaction::FilterInstalled);
    connect(r, &PackageKit::Transaction::finished, [&loop, socket, this](PackageKit::Transaction::Exit status, uint) {
        qDebug() << Q_FUNC_INFO << status;
        if (status == PackageKit::Transaction::ExitSuccess) {
            return;
        }
        socketReply(socket, QString(), 1);

        loop.quit();
    });
    connect(r, &PackageKit::Transaction::package, [&loop, socket, this](PackageKit::Transaction::Info, const QString &packageID, const QString &) {
        qDebug() << Q_FUNC_INFO << packageID;
        PackageKit::Transaction *tx = PackageKit::Daemon::removePackage(packageID, PackageKit::Transaction::TransactionFlagNone);
        connect(tx, &PackageKit::Transaction::finished, [&loop, socket, this](PackageKit::Transaction::Exit status, uint) {
            qDebug() << Q_FUNC_INFO << status;
            socketReply(socket, QString(), status == PackageKit::Transaction::ExitSuccess ? 0 : 1);

            loop.quit();
        });
    });
    loop.exec();
}

void QABridge::isAppInstalledBootstrap(QTcpSocket *socket, const QVariant &appNameArg)
{
    const QString appName = appNameArg.toString();
    qDebug() << Q_FUNC_INFO << appName;
    QEventLoop loop;
    PackageKit::Transaction *tx = PackageKit::Daemon::resolve(appName, PackageKit::Transaction::FilterInstalled);
    connect(tx, &PackageKit::Transaction::finished, [&loop, socket, this](PackageKit::Transaction::Exit status, uint) {
        qDebug() << Q_FUNC_INFO << "Status is:" << status;
        if (loop.isRunning()) {
            socketReply(socket, false);
            loop.quit();
        }
    });
    connect(tx, &PackageKit::Transaction::package, [&loop, socket, this](PackageKit::Transaction::Info, const QString &packageID, const QString &) {
        qDebug() << Q_FUNC_INFO << packageID;
        socketReply(socket, true);
        loop.quit();
    });
    loop.exec();
}

void QABridge::queryAppStateBootstrap(QTcpSocket *socket, const QVariant &appId)
{
    const QString appName = appId.toString();
    qDebug() << Q_FUNC_INFO << appName;
    QEventLoop loop;
    PackageKit::Transaction *tx = PackageKit::Daemon::resolve(appName, PackageKit::Transaction::FilterInstalled);
    connect(tx, &PackageKit::Transaction::finished, [&loop, socket, this](PackageKit::Transaction::Exit status, uint) {
        qDebug() << Q_FUNC_INFO << "Status is:" << status;
        if (loop.isRunning()) {
            socketReply(socket, QStringLiteral("NOT_INSTALLED"));
            loop.quit();
        }
    });
    connect(tx, &PackageKit::Transaction::package, [&loop, appName, socket, this](PackageKit::Transaction::Info, const QString &packageID, const QString &) {
        qDebug() << Q_FUNC_INFO << packageID;
        if (m_appPort.value(appName, 0) == 0) {
            qDebug() << Q_FUNC_INFO << m_appPort.value(appName) << "AppState is: NOT_RUNNING";
            socketReply(socket, QStringLiteral("NOT_RUNNING"));
            loop.quit();
        }

        QJsonObject json;
        json.insert(QStringLiteral("cmd"), QJsonValue(QStringLiteral("action")));
        json.insert(QStringLiteral("action"), QJsonValue(QStringLiteral("queryAppState")));
        json.insert(QStringLiteral("params"), QJsonValue::fromVariant(QStringList({appName})));
        forwardToApp(socket, QJsonDocument(json).toJson(QJsonDocument::Compact));
        loop.quit();
    });
    loop.exec();
}

void QABridge::launchAppBootstrap(QTcpSocket *socket)
{
    const QString appName = m_appSocket.value(socket);
    qDebug() << Q_FUNC_INFO << appName;
    QStringList arguments;
    ContentAction::Action action = ContentAction::Action::launcherAction(QStringLiteral("%1.desktop").arg(appName), arguments);
    action.trigger();
    socketReply(socket, QString());
}

void QABridge::closeAppBootstrap(QTcpSocket *socket)
{
    qDebug() << Q_FUNC_INFO;
    const QString appName = m_appSocket.value(socket);
    if (m_appPort.value(appName) != 0) {
        QJsonObject json;
        json.insert(QStringLiteral("cmd"), QJsonValue(QStringLiteral("action")));
        json.insert(QStringLiteral("action"), QJsonValue(QStringLiteral("closeApp")));
        json.insert(QStringLiteral("params"), QJsonValue::fromVariant(QStringList({appName})));
        forwardToApp(socket, QJsonDocument(json).toJson(QJsonDocument::Compact));
        m_appPort.insert(appName, 0);
    } else {
        qWarning() << Q_FUNC_INFO << "App" << appName << "is not active";
        socketReply(socket, QString(), 1);
    }
}

void QABridge::resetBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::setNetworkConnectionBootstrap(QTcpSocket *socket, const QVariant &connectionType)
{
    const int networkConnectionType = connectionType.toInt();
    qDebug() << Q_FUNC_INFO << networkConnectionType;

    NetworkManager* nm = NetworkManager::instance();
    if (nm->getTechnologies().isEmpty()) {
        QEventLoop loop;
        connect(nm, &NetworkManager::technologiesChanged, &loop, &QEventLoop::quit);
        loop.exec();
    }

    qDebug() << Q_FUNC_INFO << "Offline:" << nm->offlineMode();

    if (!nm->offlineMode() && (networkConnectionType & NetworkConnectionAirplane) == NetworkConnectionAirplane) {
        qDebug() << "SetOffline";
        nm->setOfflineMode(true);
        socketReply(socket, QString());
        return;
    }

    if (nm->offlineMode() && (networkConnectionType & NetworkConnectionAirplane) == 0) {
        qDebug() << "SetOnline";
        nm->setOfflineMode(false);

        QEventLoop loop;
        connect(nm, &NetworkManager::stateChanged, &loop, &QEventLoop::quit);
        loop.exec();
    }

    NetworkTechnology* wifiTech = nm->getTechnology(QStringLiteral("wifi"));
    if (wifiTech) {
        qDebug() << "Wifi powered:" << wifiTech->powered();
        wifiTech->setPowered((networkConnectionType & NetworkConnectionWifi) == NetworkConnectionWifi);
    } else {
        qDebug() << "Wifi not available";
    }

    NetworkTechnology* cellularTech = nm->getTechnology(QStringLiteral("cellular"));
    if (cellularTech) {
        qDebug() << "Data powered:" << cellularTech->powered();
        cellularTech->setPowered((networkConnectionType & NetworkConnectionData) == NetworkConnectionData);
    } else {
        qDebug() << "Data not available";
    }

    socketReply(socket, QString());
}

void QABridge::getNetworkConnectionBootstrap(QTcpSocket *socket)
{
    socketReply(socket, getNetworkConnection());
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
    const QString dateFormat = format.toString();
    const QString time = dateFormat.isEmpty()
            ? QDateTime::currentDateTime().toString(Qt::TextDate)
            : QDateTime::currentDateTime().toString(dateFormat);
    socketReply(socket, time);
}

void QABridge::startRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments)
{
    socketReply(socket, QString());
}

void QABridge::stopRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments)
{
    socketReply(socket, QString());
}

void QABridge::executeBootstrap(QTcpSocket *socket, const QVariant &commandArg, const QVariant &paramsArg)
{
    const QString command = commandArg.toString();
    const QStringList commandPair = command.split(QChar(':'));
    if (commandPair.length() != 2) {
        socketReply(socket, QString());
        return;
    }
    if (commandPair.first() != QStringLiteral("system")) {
        QJsonObject json;
        json.insert(QStringLiteral("cmd"), QJsonValue(QStringLiteral("action")));
        json.insert(QStringLiteral("action"), QJsonValue(QStringLiteral("execute")));
        json.insert(QStringLiteral("params"), QJsonValue::fromVariant(QVariantList({command, paramsArg})));
        forwardToApp(socket, QJsonDocument(json).toJson(QJsonDocument::Compact));
        return;
    }

    qDebug() << Q_FUNC_INFO << socket << command << paramsArg;

    const QVariantList params = paramsArg.toList();
    QGenericArgument arguments[9] = { QGenericArgument() };
    for (int i = 0; i < params.length(); i++) {
        arguments[i] = Q_ARG(QVariant, params[i]);
    }

    bool handled = QMetaObject::invokeMethod(this,
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

void QABridge::executeAsyncBootstrap(QTcpSocket *socket, const QVariant &commandArg, const QVariant &paramsArg)
{
    const QString command = commandArg.toString();
    const QStringList commandPair = command.split(QChar(':'));
    if (commandPair.length() != 2) {
        socketReply(socket, QString());
        return;
    }
    if (commandPair.first() != QStringLiteral("system")) {
        QJsonObject json;
        json.insert(QStringLiteral("cmd"), QJsonValue(QStringLiteral("action")));
        json.insert(QStringLiteral("action"), QJsonValue(QStringLiteral("executeAsync")));
        json.insert(QStringLiteral("params"), QJsonValue::fromVariant(QVariantList({command, paramsArg})));
        forwardToApp(socket, QJsonDocument(json).toJson(QJsonDocument::Compact));
    }
}

void QABridge::executeCommand_shell(QTcpSocket *socket, const QVariant &executableArg, const QVariant &paramsArg)
{
    const QString executable = executableArg.toString();
    const QStringList params = paramsArg.toStringList();

    QProcess p;
    p.start(executable, params);
    p.waitForFinished();

    const QString stdout = QString::fromUtf8(p.readAllStandardOutput());
    const QString stderr = QString::fromUtf8(p.readAllStandardError());
    qDebug() << Q_FUNC_INFO << "stdout:" << stdout;
    qDebug() << Q_FUNC_INFO << "stderr:" << stderr;
    socketReply(socket, stdout);
}

void QABridge::ApplicationReady(const QString &appName)
{
    qDebug() << Q_FUNC_INFO << appName;

    if (!m_appPort.contains(appName)) {
        return;
    }

    connectAppSocket(appName);
}

void QABridge::processCommand(QTcpSocket *socket, const QByteArray &cmd)
{
    qDebug() << Q_FUNC_INFO << cmd;
    QJsonParseError error;
    QJsonDocument json = QJsonDocument::fromJson(cmd, &error);
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
    const QVariantList params = object.value(QStringLiteral("params")).toVariant().toList();

    const QString methodName = QStringLiteral("%1Bootstrap").arg(action);

    for (int i = metaObject()->methodOffset(); i < metaObject()->methodOffset() + metaObject()->methodCount(); i++) {
        if (metaObject()->method(i).name() == methodName.toLatin1()) {
            QGenericArgument arguments[9] = { QGenericArgument() };
            for (int i = 0; i < params.length(); i++) {
                arguments[i] = Q_ARG(QVariant, params[i]);
            }
            QMetaObject::invokeMethod(this,
                                      methodName.toLatin1().constData(),
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
            return;
        }
    }

    qDebug() << Q_FUNC_INFO << "Process command is finished for:" << action << m_appSocket.contains(socket);

    QMetaObject::invokeMethod(this,
                              "forwardToApp",
                              Qt::DirectConnection,
                              Q_ARG(QTcpSocket*, socket),
                              Q_ARG(QByteArray, cmd));
}

void QABridge::getCurrentContextBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QStringLiteral("NATIVE_APP"));
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

void QABridge::lockBootstrap(QTcpSocket *socket, const QVariant &secondsArg)
{
    QDBusMessage lock = QDBusMessage::createMethodCall(
                QStringLiteral("com.nokia.mce"),
                QStringLiteral("/com/nokia/mce/request"),
                QStringLiteral("com.nokia.mce.request"),
                QStringLiteral("req_display_state_off"));
    QDBusReply<void> reply = QDBusConnection::systemBus().call(lock);

    if (reply.error().type() == QDBusError::NoError) {
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QABridge::pushFileBootstrap(QTcpSocket *socket, const QVariant &pathArg, const QVariant &dataArg)
{
    qDebug() << Q_FUNC_INFO << pathArg;
    const QByteArray data = dataArg.toByteArray();
    QFile file(pathArg.toString());

    if (!file.open(QFile::WriteOnly)) {
        socketReply(socket, QString(), 1);
    }
    bool success = file.write(QByteArray::fromBase64(data));

    if (!success) {
        qWarning() << Q_FUNC_INFO << "Error saving file!";
        socketReply(socket, QString(), 1);
        return;
    }

    if (data.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Saved empty file!";
    }

    socketReply(socket, QString());
}

void QABridge::pullFileBootstrap(QTcpSocket *socket, const QVariant &pathArg)
{
    qDebug() << Q_FUNC_INFO << pathArg;
    QFile file(pathArg.toString());

    if (!file.open(QFile::ReadOnly)) {
        qWarning() << Q_FUNC_INFO << "Can't find file" << pathArg.toString() << file.errorString();
        socketReply(socket, QString(), 1);
    } else {
        const QByteArray data = file.readAll().toBase64();
        qDebug() << Q_FUNC_INFO << data << "Base64 is" << QByteArray::fromBase64(data);
        socketReply(socket, data);
    }
}

void QABridge::pullFolderBootstrap(QTcpSocket *socket, const QVariant &pathArg)
{
    const QString sourceFolder = pathArg.toString();
    QByteArray reply;

    QDir src(sourceFolder);
    if(!src.exists())//folder not found
    {
        qWarning() << Q_FUNC_INFO << "Can't finde source path" << sourceFolder;
        socketReply(socket, QString(), 1);
        return;
    }

    bool success = compressFolder(sourceFolder, "");
    qDebug() << Q_FUNC_INFO << "Wait operation compressFolder";

    if (success) {
        qDebug() << Q_FUNC_INFO << "compressFolder finished" << success << QByteArray::fromBase64(m_dataStream);
        socketReply(socket, reply);
    }
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
    qDebug() << Q_FUNC_INFO;
    QDBusMessage unlock = QDBusMessage::createMethodCall(
                QStringLiteral("com.nokia.mce"),
                QStringLiteral("/com/nokia/mce/request"),
                QStringLiteral("com.nokia.mce.request"),
                QStringLiteral("req_display_state_on"));

    QDBusReply<void> displayOnReply = QDBusConnection::systemBus().call(unlock);
    if (displayOnReply.error().type() != QDBusError::NoError) {
        socketReply(socket, QString(), 1);
        return;
    }

    QDBusMessage changeLockMode = QDBusMessage::createMethodCall(
                QStringLiteral("com.nokia.mce"),
                QStringLiteral("/com/nokia/mce/request"),
                QStringLiteral("com.nokia.mce.request"),
                QStringLiteral("req_tklock_mode_change"));
    changeLockMode.setArguments({"unlocked"});
    QDBusReply<void> unlockReply = QDBusConnection::systemBus().call(changeLockMode);
    if (unlockReply.error().type() == QDBusError::NoError) {
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QABridge::isLockedBootstrap(QTcpSocket *socket)
{
    qDebug() << Q_FUNC_INFO;
    QDBusMessage lock = QDBusMessage::createMethodCall(
                QStringLiteral("com.nokia.mce"),
                QStringLiteral("/com/nokia/mce/request"),
                QStringLiteral("com.nokia.mce.request"),
                QStringLiteral("get_tklock_mode"));
    QDBusReply<QString> reply = QDBusConnection::systemBus().call(lock);

    qDebug() << Q_FUNC_INFO << reply.value() << reply;
    if (reply.error().type() == QDBusError::NoError) {
        socketReply(socket, reply.value() == QStringLiteral("locked"));
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QABridge::forwardToApp(QTcpSocket *socket, const QByteArray &data)
{
    if (!m_appSocket.contains(socket)) {
        return;
    }

    const QString appName = m_appSocket.value(socket);

    if (!m_appPort.contains(appName)) {
        qWarning() << Q_FUNC_INFO << "Unknown app:" << socket;
        return;
    }

    QTcpSocket appSocket;
    appSocket.connectToHost(QHostAddress(QHostAddress::LocalHost), m_appPort.value(appName));
    if (!appSocket.isOpen()) {
        qWarning() << Q_FUNC_INFO << "Can't connect to app socket:" << m_appPort.value(appName);
    }
    qWarning() << Q_FUNC_INFO << "Writing to app socket:" <<
    appSocket.write(data) <<
    appSocket.waitForBytesWritten();

    qDebug() << Q_FUNC_INFO << "Reading from app socket";
    QByteArray appReplyData;
    while (appSocket.state() == QTcpSocket::ConnectedState) {
        appSocket.waitForReadyRead(3000);
        appReplyData.append(appSocket.readAll());
    }
    qDebug() << Q_FUNC_INFO << appReplyData;

    socket->write(appReplyData);
    qWarning() << Q_FUNC_INFO << "Writing to appium socket:" <<
                  socket->waitForBytesWritten();
}

void QABridge::connectAppSocket(const QString &appName)
{
    QDBusMessage startAppSocket = QDBusMessage::createMethodCall(
                QStringLiteral("ru.omprussia.qaservice.%1").arg(appName),
                QStringLiteral("/ru/omprussia/qaservice"),
                QStringLiteral("ru.omprussia.qaservice"),
                QStringLiteral("startSocket"));
    QDBusReply<int> reply = QDBusConnection::sessionBus().call(startAppSocket);
    m_appPort.insert(appName, reply.value());
    qDebug() << Q_FUNC_INFO << appName << reply.value();

    if (m_appPort.value(appName) != 0 && m_connectLoop->isRunning()) {
        m_connectLoop->quit();
    }
}

int QABridge::getNetworkConnection() const
{
    NetworkManager* nm = NetworkManager::instance();
    if (nm->getTechnologies().isEmpty()) {
        QEventLoop loop;
        connect(nm, &NetworkManager::technologiesChanged, &loop, &QEventLoop::quit);
        loop.exec();
    }
    if (nm->offlineMode()) {
        return NetworkConnectionAirplane;
    }
    int connection = NetworkConnectionNone;
    for (NetworkTechnology* tech : nm->getTechnologies()) {
        if (tech->powered() && tech->type() == QStringLiteral("wifi")) {
            connection |= NetworkConnectionWifi;
        }
        if (tech->powered() && tech->type() == QStringLiteral("cellular")) {
            connection |= NetworkConnectionData;
        }
    }
    return connection;
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

bool QABridge::compressFolder(QString sourceFolder, QString prefex)
{
    QDir dir(sourceFolder);
    if(!dir.exists())
        return false;

    //1 - list all folders inside the current folder
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
    QFileInfoList foldersList = dir.entryInfoList();

    qDebug() << Q_FUNC_INFO << "Folderlist lenght is:" << foldersList.length();

    //2 - For each folder in list: call the same function with folders' paths
    for(int i=0; i<foldersList.length(); i++)
    {
        QString folderName = foldersList.at(i).fileName();
        QString folderPath = dir.absolutePath()+"/"+folderName;
        QString newPrefex = prefex+"/"+folderName;

        qDebug() << Q_FUNC_INFO << folderName << folderPath << newPrefex;
        compressFolder(folderPath, newPrefex);
    }

    //3 - List all files inside the current folder
    dir.setFilter(QDir::NoDotAndDotDot | QDir::Files);
    QFileInfoList filesList = dir.entryInfoList();

    //4- For each file in list: add file path and compressed binary data
    for(int i=0; i<filesList.length(); i++)
    {
        QString fileName = dir.absolutePath()+"/"+filesList.at(i).fileName();
        QFile file(dir.absolutePath()+"/"+filesList.at(i).fileName());
        qDebug() << Q_FUNC_INFO << filesList.length() << fileName;
        if(!file.open(QIODevice::ReadOnly))//couldn't open file
        {
            qDebug() << Q_FUNC_INFO << "Couldn't open file";
            return false;
        }

        m_dataStream.append(QString(prefex+"/"+filesList.at(i).fileName()));
        m_dataStream.append(qCompress(file.readAll()));

        qDebug() << Q_FUNC_INFO << "Compress binary data" << QByteArray::fromBase64(file.readAll());

        file.close();
    }
    return true;
}


