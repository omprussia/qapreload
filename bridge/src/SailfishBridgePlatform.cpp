#include "SailfishBridgePlatform.hpp"
#include "QAScreenRecorder.hpp"

#include <Transaction>
#include <Daemon>

#include <rpm/rpmlib.h>
#include <rpm/header.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmts.h>

#include <connman-qt5/networkmanager.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

#include <QDebug>

namespace {

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

}

SailfishBridgePlatform::SailfishBridgePlatform(QObject *parent)
    : LinuxBridgePlatform(parent)
{
    qDebug()
        << Q_FUNC_INFO;
}

void SailfishBridgePlatform::installAppCommand(QTcpSocket *socket, const QString &appPath)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appPath;

    QEventLoop loop;
    PackageKit::Transaction *tx = PackageKit::Daemon::installFile(appPath, PackageKit::Transaction::TransactionFlagNone);
    connect(tx, &PackageKit::Transaction::finished, [&loop, socket, this](PackageKit::Transaction::Exit status, uint) {
        qDebug()
            << Q_FUNC_INFO
            << status;
        socketReply(socket, QString(), status == PackageKit::Transaction::ExitSuccess ? 0 : 1);

        loop.quit();
    });
    loop.exec();

    socketReply(socket, QString(), 0);
}

void SailfishBridgePlatform::removeAppCommand(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;

    QEventLoop loop;
    PackageKit::Transaction *r = PackageKit::Daemon::resolve(appName, PackageKit::Transaction::FilterInstalled);
    connect(r, &PackageKit::Transaction::finished, [&loop, socket, this](PackageKit::Transaction::Exit status, uint) {
        qDebug()
            << Q_FUNC_INFO
            << status;
        if (status == PackageKit::Transaction::ExitSuccess) {
            return;
        }
        socketReply(socket, QString(), 1);

        loop.quit();
    });
    connect(r, &PackageKit::Transaction::package, [&loop, socket, this](PackageKit::Transaction::Info, const QString &packageID, const QString &) {
        qDebug()
            << Q_FUNC_INFO
            << packageID;
        PackageKit::Transaction *tx = PackageKit::Daemon::removePackage(packageID, PackageKit::Transaction::TransactionFlagNone);
        connect(tx, &PackageKit::Transaction::finished, [&loop, socket, this](PackageKit::Transaction::Exit status, uint) {
            qDebug()
                << Q_FUNC_INFO
                << status;
            socketReply(socket, QString(), status == PackageKit::Transaction::ExitSuccess ? 0 : 1);

            loop.quit();
        });
    });
    loop.exec();
    socketReply(socket, QString(), 0);
}

void SailfishBridgePlatform::isAppInstalledCommand(QTcpSocket *socket, const QString &rpmName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << rpmName;

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

    socketReply(socket, isInstalled);
}

void SailfishBridgePlatform::lockCommand(QTcpSocket *socket, double seconds)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << seconds;

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

    socketReply(socket, QStringLiteral("not implemented"), 1);
}

void SailfishBridgePlatform::unlockCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

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

void SailfishBridgePlatform::isLockedCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

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
}

void SailfishBridgePlatform::setNetworkConnectionCommand(QTcpSocket *socket, double connectionType)
{
    const int networkConnectionType = static_cast<int>(connectionType);
    qDebug()
        << Q_FUNC_INFO
        << socket << networkConnectionType;

    NetworkManager *nm = NetworkManager::instance();
    if (nm->getTechnologies().isEmpty()) {
        QEventLoop loop;
        connect(nm, &NetworkManager::technologiesChanged, &loop, &QEventLoop::quit);
        loop.exec();
    }

    qDebug()
        << Q_FUNC_INFO
        << "Offline:" << nm->offlineMode();

    if (!nm->offlineMode() && (networkConnectionType & NetworkConnectionAirplane) == NetworkConnectionAirplane) {
        qDebug()
            << Q_FUNC_INFO
            << "SetOffline";
        nm->setOfflineMode(true);
        socketReply(socket, QString());
        return;
    }

    if (nm->offlineMode() && (networkConnectionType & NetworkConnectionAirplane) == 0) {
        qDebug()
            << Q_FUNC_INFO
            << "SetOnline";
        nm->setOfflineMode(false);

        QEventLoop loop;
        connect(nm, &NetworkManager::stateChanged, &loop, &QEventLoop::quit);
        loop.exec();
    }

    NetworkTechnology *wifiTech = nm->getTechnology(QStringLiteral("wifi"));
    if (wifiTech) {
        qDebug()
            << Q_FUNC_INFO
            << "Wifi powered:" << wifiTech->powered();
        wifiTech->setPowered((networkConnectionType & NetworkConnectionWifi) == NetworkConnectionWifi);
    } else {
        qDebug()
            << Q_FUNC_INFO
            << "Wifi not available";
    }

    NetworkTechnology *cellularTech = nm->getTechnology(QStringLiteral("cellular"));
    if (cellularTech) {
        qDebug()
            << Q_FUNC_INFO
            << "Data powered:" << cellularTech->powered();
        cellularTech->setPowered((networkConnectionType & NetworkConnectionData) == NetworkConnectionData);
    } else {
        qDebug()
            << Q_FUNC_INFO
            << "Data not available";
    }

    socketReply(socket, QString());
}

void SailfishBridgePlatform::getNetworkConnectionCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    NetworkManager *nm = NetworkManager::instance();
    if (nm->getTechnologies().isEmpty()) {
        QEventLoop loop;
        connect(nm, &NetworkManager::technologiesChanged, &loop, &QEventLoop::quit);
        loop.exec();
    }
    if (nm->offlineMode()) {
        socketReply(socket, NetworkConnectionAirplane);
        return;
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

    socketReply(socket, connection);
}

void SailfishBridgePlatform::startRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << arguments;

    if (!m_screenrecorder) {
        m_screenrecorder = new QAScreenRecorder(this);
    }

    m_screenrecorder->setScale(0.8);
    if (!m_screenrecorder->start()) {
        socketReply(socket, QStringLiteral("error"), 1);
        return;
    }

    socketReply(socket, QStringLiteral("started"));
}

void SailfishBridgePlatform::stopRecordingScreenCommand(QTcpSocket *socket, const QVariant &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << arguments;

    if (!m_screenrecorder) {
        m_screenrecorder = new QAScreenRecorder(this);
    }

    if (!m_screenrecorder->stop()) {
        socketReply(socket, QStringLiteral("error"), 1);
        return;
    }

    socketReply(socket, m_screenrecorder->lastFilename());
}

void SailfishBridgePlatform::executeCommand_system_unlock(QTcpSocket *socket, const QVariant &executableArg, const QVariant &paramsArg)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << executableArg << paramsArg;

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

bool SailfishBridgePlatform::lauchAppStandalone(const QString &appName, const QStringList &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << appName << arguments;

    QDBusMessage launch = QDBusMessage::createMethodCall(QStringLiteral("ru.omprussia.qaservice"),
                                                         QStringLiteral("/ru/omprussia/qaservice"),
                                                         QStringLiteral("ru.omprussia.qaservice"),
                                                         QStringLiteral("launchApp"));
    launch.setArguments({ appName, arguments });
    return getSessionBus().send(launch);

}
