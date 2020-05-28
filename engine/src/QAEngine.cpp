// Copyright (c) 2020 Open Mobile Platform LLС.
#include "QAEngine.hpp"

#include <QDebug>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QFileInfo>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QMetaMethod>

#if defined Q_OS_SAILFISH
#include "SailfishEnginePlatform.hpp"
#else
#include <QApplication>
#include "QuickEnginePlatform.hpp"
#include "WidgetsEnginePlatform.hpp"
#endif

#include "QAEngineSocketClient.hpp"

namespace {

inline QGenericArgument qVariantToArgument(const QVariant &variant) {
    if (variant.isValid() && !variant.isNull()) {
        return QGenericArgument(variant.typeName(), variant.constData());
    }
    return QGenericArgument();
}

QAEngine *s_instance = nullptr;

QString s_processName;

}

bool QAEngine::isLoaded()
{
    return s_instance;
}

void QAEngine::initialize()
{
    qDebug()
        << Q_FUNC_INFO;

    setParent(qGuiApp);

#if defined Q_OS_SAILFISH
    m_platform = new SailfishEnginePlatform(this);
#else
    QApplication *wapp = qobject_cast<QApplication*>(qGuiApp);
    if (wapp) {
        m_platform = new WidgetsEnginePlatform(this);
    } else {
        m_platform = new QuickEnginePlatform(this);
    }
#endif

    connect(m_platform, &IEnginePlatform::ready, this, &QAEngine::onPlatformReady);
}

void QAEngine::onPlatformReady()
{
    qDebug()
        << Q_FUNC_INFO;
    s_processName = QFileInfo(qApp->applicationFilePath()).baseName();

    m_client = new QAEngineSocketClient(this);
    connect(m_client, &QAEngineSocketClient::commandReceived,
            this, &QAEngine::processCommand);
    m_client->connectToBridge();
}

QAEngine *QAEngine::instance()
{
    if (!s_instance) {
        s_instance = new QAEngine;
    }
    return s_instance;
}

QAEngine::QAEngine(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QTcpSocket*>();
}

QAEngine::~QAEngine()
{
}

QString QAEngine::processName()
{
    return s_processName;
}

bool QAEngine::metaInvoke(QTcpSocket *socket, QObject *object, const QString &methodName, const QVariantList &params, bool *implemented)
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
            }
        }
    } while ((mo = mo->superClass()));

    if (implemented) {
        *implemented = false;
    }
    return false;
}

void QAEngine::processCommand(QTcpSocket *socket, const QByteArray &cmd)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    qDebug().noquote() << cmd;

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

    processAppiumCommand(socket, action, params);
}

bool QAEngine::processAppiumCommand(QTcpSocket *socket, const QString &action, const QVariantList &params)
{
    const QString methodName = QStringLiteral("%1Command").arg(action);
    qDebug()
        << Q_FUNC_INFO
        << socket << methodName << params;

    bool implemented = true;
    bool result = metaInvoke(socket, m_platform, methodName, params, &implemented);

    if (!implemented) {
        m_platform->socketReply(socket, QStringLiteral("not_implemented"), 405);
    }

    return result;
}
