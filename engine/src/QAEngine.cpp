// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAEngine.hpp"

#include <QDebug>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QFileInfo>
#include <QJsonDocument>
#include <QTcpSocket>
#include <QMetaMethod>
#include <QWindow>
#include <QTimer>
#include <QQuickWindow>
#include <QDir>
#include <QDateTime>

#if defined Q_OS_SAILFISH
#include "SailfishEnginePlatform.hpp"
#else
#include <QApplication>
#include "QuickEnginePlatform.hpp"
#include "WidgetsEnginePlatform.hpp"
#endif

#include "QAEngineSocketClient.hpp"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(categoryEngine, "omp.qaengine.engine", QtWarningMsg)

namespace {

QAEngine *s_instance = nullptr;

QString s_processName = "qaengine";

QHash<QWindow*, IEnginePlatform*> s_windows;
QWindow *s_lastFocusWindow = nullptr;

inline QGenericArgument qVariantToArgument(const QVariant &variant) {
    if (variant.isValid() && !variant.isNull()) {
        return QGenericArgument(variant.typeName(), variant.constData());
    }
    return QGenericArgument();
}

void fileOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QFile logFile;
    logFile.setFileName(s_processName + QLatin1String(".log"));
    logFile.open(QFile::WriteOnly | QFile::Append);
    if (!logFile.isOpen()) {
        return;
    }
    QString log = QStringLiteral("%3 [%2]: %1\n")
            .arg(msg)
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("hh:mm:ss")));
    switch (type) {
    case QtDebugMsg:
        log = log.arg(QStringLiteral("D"));
        break;
    case QtInfoMsg:
        log = log.arg(QStringLiteral("I"));
        break;
    case QtWarningMsg:
        log = log.arg(QStringLiteral("W"));
        break;
    case QtCriticalMsg:
        log = log.arg(QStringLiteral("C"));
        break;
    case QtFatalMsg:
        log = log.arg(QStringLiteral("F"));
    }
    logFile.write(log.toUtf8());
}

}

bool QAEngine::isLoaded()
{
    return s_instance;
}

void QAEngine::objectCreated(QObject *o)
{
    if (!s_instance) {
        return;
    }

    s_instance->addItem(o);
}

void QAEngine::objectRemoved(QObject *o)
{
    if (!s_instance) {
        return;
    }

    s_instance->removeItem(o);
}

IEnginePlatform *QAEngine::getPlatform(bool silent)
{
    if (s_windows.contains(s_lastFocusWindow)) {
        return s_windows.value(s_lastFocusWindow);
    } else {
        if (!silent) {
            qCWarning(categoryEngine)
                << Q_FUNC_INFO
                << "No platform for window:" << s_lastFocusWindow;
        }
    }
    return nullptr;
}

void QAEngine::initialize()
{
    qCDebug(categoryEngine)
        << Q_FUNC_INFO
        << qApp->arguments().first();

    qCDebug(categoryEngine)
        << "Version:"
#ifdef QAPRELOAD_VERSION
        << QStringLiteral(QAPRELOAD_VERSION);
#else
        << QStringLiteral("2.0.0-dev");
#endif

    setParent(qGuiApp);

    connect(qGuiApp, &QGuiApplication::focusWindowChanged, this, &QAEngine::onFocusWindowChanged);
    if (!qGuiApp->topLevelWindows().isEmpty()) {
        onFocusWindowChanged(qGuiApp->topLevelWindows().first());
    }
    if (!qGuiApp->focusWindow()) {
        QTimer *t = new QTimer();
        connect(t, &QTimer::timeout, [this, t]() {
            if (!qGuiApp->focusWindow()) {
                return;
            }
            onFocusWindowChanged(qGuiApp->focusWindow());
            t->deleteLater();
        });
        t->start(1000);
    }
}

void QAEngine::onFocusWindowChanged(QWindow *window)
{
    qCDebug(categoryEngine)
        << "focusWindowChanged" << window
        << "appName" << qApp->arguments().first();

    if (!window) {
        return;
    }

    if (!s_windows.contains(window)) {
        IEnginePlatform *platform = nullptr;
#if defined Q_OS_SAILFISH
        platform = new SailfishEnginePlatform(window);
#else
        if (qobject_cast<QQuickWindow*>(window)) {
            platform = new QuickEnginePlatform(window);
        } else {
            platform = new WidgetsEnginePlatform(window);
        }
#endif
        connect(platform, &IEnginePlatform::ready, this, &QAEngine::onPlatformReady);
        connect(window, &QWindow::destroyed, [window]() {
            s_windows.remove(window);
        });
        QTimer::singleShot(0, platform, &IEnginePlatform::initialize);

        s_windows.insert(window, platform);

#ifdef Q_OS_WIN
        qInstallMessageHandler(fileOutput);
#endif
    }

    s_lastFocusWindow = window;
}

void QAEngine::onPlatformReady()
{
    IEnginePlatform *platform = qobject_cast<IEnginePlatform*>(sender());
    if (!platform) {
        return;
    }

    s_processName = QFileInfo(qApp->arguments().first()).baseName();;
    qCDebug(categoryEngine)
        << Q_FUNC_INFO
        << "Process name:" << s_processName
        << "platform:" << platform << platform->window() << platform->rootObject();

    if (!m_client) {
        m_client = new QAEngineSocketClient(this);
        connect(m_client, &QAEngineSocketClient::commandReceived,
                this, &QAEngine::processCommand);
    }
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

    if (QDir::home().exists(QStringLiteral(".qapreload-logging"))) {
        QLoggingCategory::setFilterRules("omp.qaengine.*.debug=true");
    } else {
        QLoggingCategory::setFilterRules("omp.qaengine.*.warning=true");
    }
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
                qCDebug(categoryEngine)
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

void QAEngine::addItem(QObject *o)
{
    if (auto platform = getPlatform(true)) {
        platform->addItem(o);
    }
}

void QAEngine::removeItem(QObject *o)
{
    if (auto platform = getPlatform(true)) {
        platform->removeItem(o);
    }
    if (s_lastFocusWindow == o) {
        s_lastFocusWindow = qGuiApp->topLevelWindows().first();
    }
}

void QAEngine::processCommand(QTcpSocket *socket, const QByteArray &cmd)
{
    qCDebug(categoryEngine)
        << Q_FUNC_INFO
        << socket;
    qCDebug(categoryEngine).noquote() << cmd;

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
    qCDebug(categoryEngine)
        << Q_FUNC_INFO
        << socket << methodName << params;

    bool result = false;
    if (auto platform = getPlatform()) {
        qCDebug(categoryEngine) << Q_FUNC_INFO << platform << platform->window() << platform->rootObject();
        bool implemented = true;
        result = metaInvoke(socket, platform, methodName, params, &implemented);

        if (!implemented) {
            platform->socketReply(socket, QStringLiteral("not_implemented"), 405);
        }
    } else {
        QJsonObject reply;
        reply.insert(QStringLiteral("status"), 1);
        reply.insert(QStringLiteral("value"), QStringLiteral("no platform!"));

        const QByteArray data = QJsonDocument(reply).toJson(QJsonDocument::Compact);

        socket->write(data);
        socket->flush();
    }

    return result;
}
