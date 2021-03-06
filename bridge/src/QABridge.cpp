// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QABridge.hpp"
#if defined Q_OS_SAILFISH
#include "SailfishBridgePlatform.hpp"
#elif defined Q_OS_LINUX
#include "LinuxBridgePlatform.hpp"
#elif defined Q_OS_MACOS
#include "MacBridgePlatform.hpp"
#elif defined Q_OS_WINDOWS
#include "WindowsBridgePlatform.hpp"
#else
#include "GenericBridgePlatform.hpp"
#endif

#include "ITransportClient.hpp"
#include "TCPSocketServer.hpp"

#include <QDebug>

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMetaMethod>
#include <QTimer>

#include <QJsonObject>
#include <QJsonDocument>

namespace {

inline QGenericArgument qVariantToArgument(const QVariant &variant) {
    if (variant.isValid() && !variant.isNull()) {
        return QGenericArgument(variant.typeName(), variant.constData());
    }
    return QGenericArgument();
}

}

QABridge::QABridge(QObject *parent)
    : QObject(parent)
#if defined Q_OS_SAILFISH
    , m_platform(new SailfishBridgePlatform(this))
#elif defined Q_OS_LINUX
    , m_platform(new LinuxBridgePlatform(this))
#elif defined Q_OS_MACOS
    , m_platform(new MacBridgePlatform(this))
#elif defined Q_OS_WINDOWS
    , m_platform(new WindowsBridgePlatform(this))
#else
    , m_platform(new GenericBridgePlatform(this))
#endif
    , m_socketServer(new TCPSocketServer(8888, this))
{
    qDebug()
        << "Version:"
#ifdef QAPRELOAD_VERSION
        << QStringLiteral(QAPRELOAD_VERSION);
#else
        << QStringLiteral("2.0.0-dev");
#endif

    qRegisterMetaType<ITransportClient*>();
    qRegisterMetaType<ITransportServer*>();

    connect(m_socketServer, &ITransportServer::commandReceived,
            this, &QABridge::processCommand);
    connect(m_socketServer, &ITransportServer::clientLost,
            this, &QABridge::removeClient);
}

bool QABridge::metaInvoke(ITransportClient *client, QObject *object, const QString &methodName, const QVariantList &params, bool *implemented)
{
    auto mo = object->metaObject();
    do {
        for (int i = mo->methodOffset(); i < mo->methodOffset() + mo->methodCount(); i++) {
            if (mo->method(i).name() == methodName.toLatin1()) {
                const QMetaMethod method = mo->method(i);
                QGenericArgument arguments[9] = { QGenericArgument() };
                for (int i = 0; i < (method.parameterCount() - 1) && params.count() > i; i++) {
                    if (method.parameterType(i + 1) == QMetaType::QVariant) {
                        arguments[i] = Q_ARG(QVariant, params[i]);
                    } else {
                        arguments[i] = qVariantToArgument(params[i]);
                    }
                }
                qDebug()
                    << Q_FUNC_INFO
                    << "found" << methodName
                    << "in" << mo->className();

                if (implemented) {
                    *implemented = true;
                }

                return QMetaObject::invokeMethod(
                    object,
                    methodName.toLatin1().constData(),
                    Qt::DirectConnection,
                    Q_ARG(ITransportClient*, client),
                    arguments[0],
                    arguments[1],
                    arguments[2],
                    arguments[3],
                    arguments[4],
                    arguments[5],
                    arguments[6],
                    arguments[7],
                    arguments[8]);
            }
        }
    } while ((mo = mo->superClass()));

    if (implemented) {
        *implemented = false;
    }
    return false;
}

void QABridge::start()
{
    qDebug() << Q_FUNC_INFO;

    m_socketServer->start();
}

void QABridge::removeClient(ITransportClient *client)
{
    qDebug() << Q_FUNC_INFO << client;

    m_platform->removeClient(client);
}

void QABridge::processCommand(ITransportClient *client, const QByteArray &cmd)
{
    qDebug()
        << Q_FUNC_INFO
        << client << cmd;

    const QJsonObject object = QJsonDocument::fromJson(cmd).object();
    if (object.contains(QStringLiteral("status")) && object.contains(QStringLiteral("value"))) {
        m_platform->appReply(client, cmd);
        return;
    }

    if (object.contains(QStringLiteral("appConnect"))) {
        const QJsonObject app = object.value(QStringLiteral("appConnect")).toObject();
        processAppConnectCommand(client, app);
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

    if (!processAppiumCommand(client, action, params)) {
        qDebug()
            << Q_FUNC_INFO
            << "Forwarding appium command to app:" << client;

        QMetaObject::invokeMethod(
            m_platform,
            "forwardToApp",
            Qt::DirectConnection,
            Q_ARG(ITransportClient*, client),
            Q_ARG(QByteArray, cmd));
    }
}

void QABridge::processAppConnectCommand(ITransportClient *client, const QJsonObject &app)
{
    qDebug()
        << Q_FUNC_INFO
        << client << app;

    const QString appName = app.value(QStringLiteral("appName")).toString();
    m_platform->appConnect(client, appName);
}

bool QABridge::processAppiumCommand(ITransportClient *client, const QString &action, const QVariantList &params)
{
    const QString methodName = QStringLiteral("%1Command").arg(action);
    qDebug()
        << Q_FUNC_INFO
        << client << methodName << params;

    auto mo = m_platform->metaObject();
    do {
        for (int i = mo->methodOffset(); i < mo->methodOffset() + mo->methodCount(); i++) {
            if (mo->method(i).name() == methodName.toLatin1()) {
                const QMetaMethod method = mo->method(i);
                QGenericArgument arguments[9] = { QGenericArgument() };
                for (int i = 0; i < (method.parameterCount() - 1) && params.count() > i; i++) {
                    if (method.parameterType(i + 1) == QMetaType::QVariant) {
                        arguments[i] = Q_ARG(QVariant, params[i]);
                    } else {
                        arguments[i] = qVariantToArgument(params[i]);
                    }
                }
                qDebug()
                    << Q_FUNC_INFO
                    << "found" << methodName
                    << "in" << mo->className();

                return QMetaObject::invokeMethod(
                    m_platform,
                    methodName.toLatin1().constData(),
                    Qt::DirectConnection,
                    Q_ARG(ITransportClient*, client),
                    arguments[0],
                    arguments[1],
                    arguments[2],
                    arguments[3],
                    arguments[4],
                    arguments[5],
                    arguments[6],
                    arguments[7],
                    arguments[8]);
            }
        }
    } while ((mo = mo->superClass()));

    return false;
}
