#include "QABridge.hpp"

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
#include <QProcess>

#ifdef USE_DBUS
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include "QAScreenRecorder.hpp"
#endif

#ifdef USE_RPM
#include <rpm/rpmlib.h>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmts.h>
#endif

#ifdef USE_PACKAGEKIT
#include <Transaction>
#include <Daemon>
#endif

#ifdef USE_CONNMAN
#include <connman-qt5/networkmanager.h>
#endif

#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#ifdef WIN32
#include <winsock.h>
#include "WinInjector.hpp"
#else
#include <sys/socket.h>
#include <sys/un.h>
#endif

#include <unistd.h>

namespace {

#ifdef USE_DBUS
QDBusConnection getSessionBus()
{
    static const QString s_sessionBusConnection = QStringLiteral("qabridge-connection");
    static QDBusConnection s_bus = QDBusConnection::connectToBus(QDBusConnection::SessionBus, s_sessionBusConnection);
    if (!s_bus.isConnected()) {
        QEventLoop loop;
        QTimer timer;
        QObject::connect(&timer, &QTimer::timeout, [&loop, &timer](){
            QDBusConnection::disconnectFromBus(s_sessionBusConnection);
            s_bus = QDBusConnection::connectToBus(QDBusConnection::SessionBus, s_sessionBusConnection);
            if (s_bus.isConnected()) {
                timer.stop();
                loop.quit();
            }
        });
        timer.start(1000);
        loop.exec();
    }
    return s_bus;
}
#endif

inline QGenericArgument qVariantToArgument(const QVariant &variant) {
    if (variant.isValid() && !variant.isNull()) {
        return QGenericArgument(variant.typeName(), variant.constData());
    }
    return QGenericArgument();
}

}

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

#ifdef USE_SYSTEMD
    if (sd_listen_fds(0) == 1) {
        int fd = SD_LISTEN_FDS_START;
        qDebug() << Q_FUNC_INFO << "Using systemd socket descriptor:" << fd <<
                    m_server->setSocketDescriptor(fd);
    } else
#endif
    if (!m_server->listen(QHostAddress::AnyIPv4, 8888)) {
        qWarning() << Q_FUNC_INFO << m_server->errorString();
        qApp->quit();
        return;
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

    qDebug() << Q_FUNC_INFO << socket << socket->bytesAvailable();
    QByteArray requestData = socket->readAll();
    qDebug().noquote() << requestData;
    requestData.replace("}{", "}\n{"); // workaround packets join

    const QList<QByteArray> commands = requestData.split('\n');
    for (const QByteArray &cmd : commands) {
        if (cmd.isEmpty()) {
            continue;
        }
        const QString appName = m_applicationSocket.key(socket);
        if (appName.isEmpty()) {
            processCommand(socket, cmd);
        } else {
            processAppCommand(socket, appName, cmd);
        }
    }
}

void QABridge::removeSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    qDebug() << Q_FUNC_INFO << socket;
    if (m_clientSocket.contains(socket)) {
        qDebug() << Q_FUNC_INFO << m_clientSocket.value(socket);
        m_clientSocket.remove(socket);
    }
}

void QABridge::initializeBootstrap(QTcpSocket *socket, const QString &appName)
{
    qDebug() << Q_FUNC_INFO << appName << socket;

    QString name = appName;
    if (QFileInfo::exists(appName)) {
        name = QFileInfo(appName).baseName();
        m_clientFullPath.insert(socket, appName);
    }

    if (m_clientSocket.contains(socket)) {
        qWarning() << Q_FUNC_INFO << "Socket already known:" << m_clientSocket.value(socket);
    }
    m_clientSocket.insert(socket, name);
    if (appName == QLatin1String("headless")) {
        return;
    }
}

void QABridge::appConnectBootstrap(QTcpSocket *socket)
{
    const QString appName = m_clientSocket.value(socket);
    if (appName == QLatin1String("headless")) {
        socketReply(socket, QString());
        return;
    }

    if (m_applicationSocket.value(appName) != nullptr) {
        QTimer maxTimer;
        connect(&maxTimer, &QTimer::timeout, m_connectLoop, &QEventLoop::quit);
        maxTimer.start(30000);
        qDebug() << Q_FUNC_INFO << "Starting eventloop connect";
        m_connectLoop->exec();
        qDebug() << Q_FUNC_INFO << "Exiting eventloop connect";
        maxTimer.stop();
    }

    qDebug() << Q_FUNC_INFO << appName;
    socketReply(socket, QString());
}

void QABridge::appDisconnectBootstrap(QTcpSocket *socket, bool autoLaunch)
{
    const QString appName = m_clientSocket.value(socket);

    if (m_applicationSocket.value(appName) != nullptr) {
        if (autoLaunch) {
            sendToAppSocket(appName, actionData(QStringLiteral("closeApp"), QStringList({appName})));
        }

        m_clientSocket.remove(socket);
        qDebug() << Q_FUNC_INFO << appName;
    }

    socketReply(socket, QString());

    if (socket->isOpen()) {
        socket->close();
    }
}

void QABridge::startActivityBootstrap(QTcpSocket *socket, const QString &appName, const QVariantList &arguments)
{
    qDebug() << Q_FUNC_INFO << appName << arguments;

    QStringList args;
    for (const QVariant &varg : arguments) {
        args.append(varg.toString());
    }
    const bool success = QABridge::launchApp(appName, args);

    if (success) {
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QABridge::activateAppBootstrap(QTcpSocket *socket, const QString &appId)
{
    const QString appName = m_clientSocket.value(socket);
    qDebug() << Q_FUNC_INFO << appName << appId;
    if (m_applicationSocket.value(appName, nullptr) == nullptr) {
        QABridge::launchApp(appName);
    } else {
        forwardToApp(socket, QStringLiteral("activateApp"), QStringList({appName}));
    }
    socketReply(socket, QString());
}

void QABridge::terminateAppBootstrap(QTcpSocket *socket, const QString &appId)
{
    const QString appName = m_clientSocket.value(socket);
    qDebug() << Q_FUNC_INFO << appName << appId;
    if (m_applicationSocket.value(appName) != nullptr) {
        forwardToApp(socket, QStringLiteral("closeApp"), QStringList({appName}));
        m_applicationSocket.insert(appName, nullptr);
    } else {
        qWarning() << Q_FUNC_INFO << "App" << appName << "is not active";
        socketReply(socket, false, 1);
    }
}

void QABridge::installAppBootstrap(QTcpSocket *socket, const QString &appPath)
{
    qDebug() << Q_FUNC_INFO << appPath;

#ifdef USE_PACKAGEKIT
    QEventLoop loop;
    PackageKit::Transaction *tx = PackageKit::Daemon::installFile(appPath, PackageKit::Transaction::TransactionFlagNone);
    connect(tx, &PackageKit::Transaction::finished, [&loop, socket, this](PackageKit::Transaction::Exit status, uint) {
        qDebug() << Q_FUNC_INFO << status;
        socketReply(socket, QString(), status == PackageKit::Transaction::ExitSuccess ? 0 : 1);

        loop.quit();
    });
    loop.exec();
#else
    socketReply(socket, QString(), 0);
#endif
}

void QABridge::removeAppBootstrap(QTcpSocket *socket, const QString &appName)
{
    qDebug() << Q_FUNC_INFO << appName;
#ifdef USE_PACKAGEKIT
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
#else
    socketReply(socket, QString(), 0);
#endif
}

void QABridge::isAppInstalledBootstrap(QTcpSocket *socket, const QString &rpmName)
{
    qDebug() << Q_FUNC_INFO << rpmName;
    const bool isInstalled = isAppInstalled(rpmName);
    socketReply(socket, isInstalled);
}

void QABridge::queryAppStateBootstrap(QTcpSocket *socket, const QString &appName)
{
    if (!isServiceRegistered(appName)) {
        socketReply(socket, QStringLiteral("NOT_RUNNING"));
    } else if (m_applicationSocket.value(appName, nullptr) == nullptr) {
        socketReply(socket, QStringLiteral("CLOSING"));
    } else {
        forwardToApp(socket, QStringLiteral("queryAppState"), QStringList({appName}));
    }
}

void QABridge::launchAppBootstrap(QTcpSocket *socket)
{
    const QString appName = m_clientSocket.value(socket);
    qDebug() << Q_FUNC_INFO << appName << m_applicationSocket.value(appName);
    if (m_applicationSocket.value(appName, nullptr) != nullptr) {
        forwardToApp(socket, QStringLiteral("activateApp"), QStringList({appName}));
    } else {
        m_applicationSocket.insert(appName, nullptr);
        qDebug() << Q_FUNC_INFO << appName;
        if (m_clientFullPath.contains(socket)) {
            QABridge::launchApp(m_clientFullPath.value(socket));
        } else {
            QABridge::launchApp(appName);
        }

        QTimer maxTimer;
        connect(&maxTimer, &QTimer::timeout, m_connectLoop, &QEventLoop::quit);
        maxTimer.start(30000);
        qDebug() << Q_FUNC_INFO << "Starting eventloop connect";
        m_connectLoop->exec();
        qDebug() << Q_FUNC_INFO << "Exiting eventloop connect";
        maxTimer.stop();
    }

    socketReply(socket, QString());
}

void QABridge::closeAppBootstrap(QTcpSocket *socket)
{
    qDebug() << Q_FUNC_INFO;
    const QString appName = m_clientSocket.value(socket);
    if (m_applicationSocket.value(appName) != nullptr) {
        QByteArray appReplyData = sendToAppSocket(appName, actionData(QStringLiteral("closeApp"), QStringList({appName})));
        qDebug() << Q_FUNC_INFO << appReplyData;

        QEventLoop loop;
        QTimer timer;
        int counter = 0;
        connect(&timer, &QTimer::timeout, [this, &loop, &counter, appName]() {
            if (!isServiceRegistered(appName)) {
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
        qWarning() << Q_FUNC_INFO << "App" << appName << "is not active";
        socketReply(socket, QString(), 1);
    }
}

void QABridge::resetBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::setNetworkConnectionBootstrap(QTcpSocket *socket, double connectionType)
{
    const int networkConnectionType = static_cast<int>(connectionType);
    qDebug() << Q_FUNC_INFO << networkConnectionType;

#ifdef USE_CONNMAN
    NetworkManager *nm = NetworkManager::instance();
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

    NetworkTechnology *wifiTech = nm->getTechnology(QStringLiteral("wifi"));
    if (wifiTech) {
        qDebug() << "Wifi powered:" << wifiTech->powered();
        wifiTech->setPowered((networkConnectionType & NetworkConnectionWifi) == NetworkConnectionWifi);
    } else {
        qDebug() << "Wifi not available";
    }

    NetworkTechnology *cellularTech = nm->getTechnology(QStringLiteral("cellular"));
    if (cellularTech) {
        qDebug() << "Data powered:" << cellularTech->powered();
        cellularTech->setPowered((networkConnectionType & NetworkConnectionData) == NetworkConnectionData);
    } else {
        qDebug() << "Data not available";
    }
#endif

    socketReply(socket, QString());
}

void QABridge::getNetworkConnectionBootstrap(QTcpSocket *socket)
{
    socketReply(socket, getNetworkConnection());
}

void QABridge::mobileShakeBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::getSettingsBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QABridge::getDeviceTimeBootstrap(QTcpSocket *socket, const QString &dateFormat)
{
    const QString time = dateFormat.isEmpty()
            ? QDateTime::currentDateTime().toString(Qt::TextDate)
            : QDateTime::currentDateTime().toString(dateFormat);
    socketReply(socket, time);
}

void QABridge::startRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments)
{
#ifdef Q_OS_SAILFISH
    if (!m_screenrecorder) {
        m_screenrecorder = new QAScreenRecorder(this);
    }

    if (!m_screenrecorder->start()) {
        socketReply(socket, QStringLiteral("error"), 1);
        return;
    }

    socketReply(socket, QStringLiteral("started"));
#else
    socketReply(socket, QStringLiteral("not_implemented"), 1);
#endif
}

void QABridge::stopRecordingScreenBootstrap(QTcpSocket *socket, const QVariant &arguments)
{
#ifdef Q_OS_SAILFISH
    if (!m_screenrecorder) {
        m_screenrecorder = new QAScreenRecorder(this);
    }

    m_screenrecorder->setScale(0.8);
    if (!m_screenrecorder->stop()) {
        socketReply(socket, QStringLiteral("error"), 1);
        return;
    }

    socketReply(socket, m_screenrecorder->lastFilename());
#else
    socketReply(socket, QStringLiteral("not_implemented"), 1);
#endif
}

void QABridge::executeBootstrap(QTcpSocket *socket, const QString &command, const QVariant &paramsArg)
{
    const QStringList commandPair = command.split(QChar(':'));
    if (commandPair.length() != 2) {
        socketReply(socket, QString());
        return;
    }
    if (commandPair.first() != QStringLiteral("system")) {
        forwardToApp(socket, QStringLiteral("execute"), QVariantList({command, paramsArg}));
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

void QABridge::executeAsyncBootstrap(QTcpSocket *socket, const QString &command, const QVariant &paramsArg)
{
    const QStringList commandPair = command.split(QChar(':'));
    if (commandPair.length() != 2) {
        socketReply(socket, QString());
        return;
    }
    if (commandPair.first() != QStringLiteral("system")) {
        forwardToApp(socket, QStringLiteral("executeAsync"), QVariantList({command, paramsArg}));
    }
}

#ifdef Q_OS_SAILFISH
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

void QABridge::executeCommand_unlock(QTcpSocket *socket, const QVariant &executableArg, const QVariant &paramsArg)
{
    qDebug() << executableArg << paramsArg;
    QVariantList params = paramsArg.toList();
    QString socketPath;
    if (params.isEmpty()) {
        QDir askDir(QStringLiteral("/run/systemd/ask-password/"));
        if (!askDir.exists()) {
            qWarning() << Q_FUNC_INFO << askDir.absolutePath() << "not exists!";
            socketReply(socket, QStringLiteral("no dir"), 1);
            return;
        }
        QStringList sockets = askDir.entryList({QStringLiteral("sck.*")}, QDir::System);
        qDebug() << Q_FUNC_INFO << sockets;
        if (sockets.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "No sockets!";
            socketReply(socket, QStringLiteral("no sockets"), 1);
            return;
        }

        socketPath = askDir.absolutePath() + QDir::separator() + sockets.first();
    } else {
        socketPath = params.first().toString();
    }

    const QString password = executableArg.toString();

    char buf[20];
    int sd, len;
    struct sockaddr_un name;

    if (password.length() > 0) {
        buf[0] = '+';
        strncpy(buf + 1, password.toLatin1().constData(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        len = strlen(buf);
    } else {  // Assume cancelled
        buf[0] = '-';
        len = 1;
    }

    if ((sd = ::socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        qWarning() << Q_FUNC_INFO << "socket open failed";
        socketReply(socket, QStringLiteral("socket open failed"), 1);
        return;
    }

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, socketPath.toLatin1().constData(), sizeof(name.sun_path));
    name.sun_path[sizeof(name.sun_path) - 1] = '\0';

    if (::connect(sd, (struct sockaddr *)&name, SUN_LEN(&name)) != 0) {
        qWarning() << Q_FUNC_INFO << "socket connect failed";
        close(sd);
        socketReply(socket, QStringLiteral("socket connect failed"), 1);
        return;
    }

    if (send(sd, buf, len, 0) < len) {
        qWarning() << Q_FUNC_INFO << "socket send failed";
        close(sd);
        socketReply(socket, QStringLiteral("socket send failed"), 1);
        return;
    }

    close(sd);

    socketReply(socket, QString());

}
#endif

void QABridge::processCommand(QTcpSocket *socket, const QByteArray &cmd)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << cmd;

    const QJsonObject object = QJsonDocument::fromJson(cmd).object();

    if (object.contains(QStringLiteral("appConnect"))) {
        const QJsonObject app = object.value(QStringLiteral("appConnect")).toObject();
        processAppConnectCommand(socket, app);
        return;
    }

    if (!object.contains(QStringLiteral("cmd"))) {
        return;
    }

    if (object.value(QStringLiteral("cmd")).toVariant().toString() != QLatin1String("action")) {
        return;
    }

    if (!object.contains(QStringLiteral("action"))) {
        return;
    }

    const QString action = object.value(QStringLiteral("action")).toVariant().toString();
    const QVariantList params = object.value(QStringLiteral("params")).toVariant().toList();

    if (!processAppiumCommand(socket, action, params)) {
        qDebug() << Q_FUNC_INFO << "Process command is finished for:" << action << m_clientSocket.contains(socket);

        QMetaObject::invokeMethod(this,
                                  "forwardToApp",
                                  Qt::DirectConnection,
                                  Q_ARG(QTcpSocket*, socket),
                                  Q_ARG(QByteArray, cmd));
    }
}

void QABridge::processAppCommand(QTcpSocket *socket, const QString &appName, const QByteArray &cmd)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << cmd;

    const QJsonObject object = QJsonDocument::fromJson(cmd).object();

    if (object.contains(QStringLiteral("forward"))) {
        const QString forwardAppName = object.value(QStringLiteral("forward")).toString();
        if (!m_applicationSocket.contains(forwardAppName)) {
            qWarning() << Q_FUNC_INFO << "No socket for app" << forwardAppName;
            return;
        }

        QMetaObject::invokeMethod(this,
                                  "forwardToApp",
                                  Qt::DirectConnection,
                                  Q_ARG(QTcpSocket*, socket),
                                  Q_ARG(QString, forwardAppName),
                                  Q_ARG(QByteArray, cmd));

        return;
    }

    emit applicationReply(socket, appName, cmd);
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

void QABridge::getLogBootstrap(QTcpSocket *socket, const QString &type)
{
    socketReply(socket, QStringList());
}

void QABridge::setGeoLocationBootstrap(QTcpSocket *socket, const QVariant &location)
{
    socketReply(socket, QString());
}

void QABridge::lockBootstrap(QTcpSocket *socket, double seconds)
{
#ifdef Q_OS_SAILFISH
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
#else
    socketReply(socket, QStringLiteral("not implemented"), 1);
#endif
}

void QABridge::pushFileBootstrap(QTcpSocket *socket, const QString &path, const QString &data)
{
    qDebug() << Q_FUNC_INFO << path;
    QFile file(path);

    if (!file.open(QFile::WriteOnly)) {
        socketReply(socket, QString(), 1);
    }
    bool success = file.write(QByteArray::fromBase64(data.toLatin1()));

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

void QABridge::pullFileBootstrap(QTcpSocket *socket, const QString &path)
{
    qDebug() << Q_FUNC_INFO << path;
    QFile file(path);

    if (!file.open(QFile::ReadOnly)) {
        qWarning() << Q_FUNC_INFO << "Can't find file" << path << file.errorString();
        socketReply(socket, QString(), 1);
    } else {
        const QByteArray data = file.readAll().toBase64();
        qDebug() << Q_FUNC_INFO << data << "Base64 is" << QByteArray::fromBase64(data);
        socketReply(socket, data);
    }
}

void QABridge::unlockBootstrap(QTcpSocket *socket)
{
    qDebug() << Q_FUNC_INFO;
#ifdef Q_OS_SAILFISH
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
#else
    socketReply(socket, QStringLiteral("not implemented"), 1);
#endif
}

void QABridge::isLockedBootstrap(QTcpSocket *socket)
{
    qDebug() << Q_FUNC_INFO;
#ifdef Q_OS_SAILFISH
    QDBusMessage lock = QDBusMessage::createMethodCall(
                QStringLiteral("com.nokia.mce"),
                QStringLiteral("/com/nokia/mce/request"),
                QStringLiteral("com.nokia.mce.request"),
                QStringLiteral("get_tklock_mode"));
    QDBusReply<QString> reply = QDBusConnection::systemBus().call(lock);

    qDebug() << Q_FUNC_INFO << reply.value() << reply;
    if (reply.error().type() == QDBusError::NoError) {
        socketReply(socket, reply.value() == QLatin1String("locked"));
    } else {
        socketReply(socket, QString(), 1);
    }
#else
    socketReply(socket, QStringLiteral("not implemented"), 1);
#endif
}

void QABridge::forwardToApp(QTcpSocket *socket, const QByteArray &data)
{
    if (!m_clientSocket.contains(socket)) {
        return;
    }

    const QString appName = m_clientSocket.value(socket);

    forwardToApp(socket, appName, data);
}

void QABridge::forwardToApp(QTcpSocket *socket, const QString &appName, const QByteArray &data)
{
    if (!m_applicationSocket.contains(appName)) {
        qWarning() << Q_FUNC_INFO << "Unknown app:" << appName << socket;
        return;
    }

    QByteArray appReplyData = sendToAppSocket(appName, data);
    qDebug() << Q_FUNC_INFO << appReplyData;

    socket->write(appReplyData);
    qWarning()
        << Q_FUNC_INFO
        << "Writing to appium socket:"
        << socket->waitForBytesWritten();
}

void QABridge::forwardToApp(QTcpSocket *socket, const QString &action, const QVariant &params)
{
    forwardToApp(socket, actionData(action, params));
}

void QABridge::processAppConnectCommand(QTcpSocket *socket, const QJsonObject &app)
{
    if (m_clientSocket.contains(socket)) {
        qDebug() << Q_FUNC_INFO << m_clientSocket.value(socket);
        m_clientSocket.remove(socket);
    }

    const QString appName = app.value(QStringLiteral("appName")).toString();
    m_applicationSocket.insert(appName, socket);

    if (m_applicationSocket.value(appName) != nullptr && m_connectLoop->isRunning()) {
        m_connectLoop->quit();
    }
}

bool QABridge::processAppiumCommand(QTcpSocket *socket, const QString &action, const QVariantList &params)
{
    const QString methodName = QStringLiteral("%1Bootstrap").arg(action);

    for (int i = metaObject()->methodOffset(); i < metaObject()->methodOffset() + metaObject()->methodCount(); i++) {
        if (metaObject()->method(i).name() == methodName.toLatin1()) {
            const QMetaMethod method = metaObject()->method(i);
            QGenericArgument arguments[9] = { QGenericArgument() };
            for (int i = 0; i < (method.parameterCount() - 1) && params.count() > i; i++) {
                if (method.parameterType(i + 1) == QMetaType::QVariant) {
                    arguments[i] = Q_ARG(QVariant, params[i]);
                } else {
                    arguments[i] = qVariantToArgument(params[i]);
                }
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
            return true;
        }
    }

    return false;
}

QByteArray QABridge::actionData(const QString &action, const QVariant &params)
{
    QJsonObject json;
    json.insert(QStringLiteral("cmd"), QJsonValue(QStringLiteral("action")));
    json.insert(QStringLiteral("action"), QJsonValue(action));
    json.insert(QStringLiteral("params"), QJsonValue::fromVariant(params));
    return QJsonDocument(json).toJson(QJsonDocument::Compact);
}

bool QABridge::isServiceRegistered(const QString &appName)
{
    return m_applicationSocket.contains(appName);
}

bool QABridge::launchApp(const QString &appName, const QStringList &arguments)
{
    QProcess *process = new QProcess;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

#ifdef Q_OS_SAILFISH
    QDBusMessage launch = QDBusMessage::createMethodCall(QStringLiteral("ru.omprussia.qaservice"),
                                                         QStringLiteral("/ru/omprussia/qaservice"),
                                                         QStringLiteral("ru.omprussia.qaservice"),
                                                         QStringLiteral("launchApp"));
    launch.setArguments({ appName, arguments });
    return getSessionBus().send(launch);
#endif

#ifdef Q_OS_LINUX
    env.insert("LD_PRELOAD", "/usr/lib/libqapreloadhook.so");
    process->setProcessEnvironment(env);

    process->start(appName);
    process->waitForStarted();
    if (process->state() == QProcess::Running) {
        return true;
    } else return false;
#endif
#ifdef Q_OS_WINDOWS
    Injector inject;

    process->start(appName);
    process->waitForStarted();
    if (process->state() == QProcess::Running) {
        return inject.injectDll(process->processId(), "qapreloadhook.dll");
    } else return false;
#endif
}

QByteArray QABridge::sendToAppSocket(const QString &appName, const QByteArray &data)
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

    QMetaObject::Connection readyReadConnection = connect(this, &QABridge::applicationReply,
                                                          [&appReplyData, socket, &loop, &timer, &haveData]
                                                          (QTcpSocket *appSocket, const QString &appName, const QByteArray &data) {
        if (!haveData) {
            haveData = true;
            appReplyData.clear();
        }
        qDebug() << Q_FUNC_INFO << "Received bytes from app:" << appName << appSocket;
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

int QABridge::getNetworkConnection() const
{
#ifdef USE_CONNMAN
    NetworkManager *nm = NetworkManager::instance();
    if (nm->getTechnologies().isEmpty()) {
        QEventLoop loop;
        connect(nm, &NetworkManager::technologiesChanged, &loop, &QEventLoop::quit);
        loop.exec();
    }
    if (nm->offlineMode()) {
        return NetworkConnectionAirplane;
    }
    int connection = NetworkConnectionNone;
    for (NetworkTechnology *tech : nm->getTechnologies()) {
        if (tech->powered() && tech->type() == QLatin1String("wifi")) {
            connection |= NetworkConnectionWifi;
        }
        if (tech->powered() && tech->type() == QLatin1String("cellular")) {
            connection |= NetworkConnectionData;
        }
    }
    return connection;
#else
    return NetworkConnectionWifi;
#endif
}

bool QABridge::isAppInstalled(const QString &rpmName)
{
    qDebug() << Q_FUNC_INFO << rpmName;

#ifdef USE_RPM
    static bool rpmConfigRead = false;
    if (!rpmConfigRead)
    {
        rpmConfigRead = true;
        if (rpmReadConfigFiles(NULL, NULL))
        {
            qWarning()
                << Q_FUNC_INFO
                << "Error reading rpm config";
        }
    }
    rpmts transactionSet = rpmtsCreate();
    rpmdbMatchIterator it = rpmtsInitIterator(transactionSet, RPMTAG_NAME, rpmName.toLatin1().constData(), 0);

    bool isInstalled = false;
    if (rpmdbNextIterator(it)) {
        isInstalled = true;
    }
    rpmdbFreeIterator(it);
    rpmtsFree(transactionSet);

    return isInstalled;
#else
    return true;
#endif
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
