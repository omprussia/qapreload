#include "QAEngine.hpp"
#include "QAHooks.hpp"
#include "QAService.hpp"

#include <QDebug>

#include <QCoreApplication>
#include <QScopedValueRollback>
#include <QTimer>

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickView>

static QAEngine *s_instance = nullptr;
static const char *c_initDelayValue = "QA_INSPECTOR_DELAY";

void QAEngine::initialize()
{
    setParent(qApp);
    int delay = 5000;
    if (Q_UNLIKELY(qEnvironmentVariableIsSet(c_initDelayValue))) {
        bool ok = false;
        const int newDelay = qEnvironmentVariableIntValue(c_initDelayValue, &ok);
        if (!ok || newDelay < 0) {
            qWarning("The delay environment variable is not valid");
        } else {
            delay = newDelay;
        }
    }
    QTimer::singleShot(delay, this, &QAEngine::postInit);
}

void QAEngine::postInit()
{
    m_rootItem = findRootItem();
    qDebug() << Q_FUNC_INFO << "Root item:" << m_rootItem;
    if (m_rootItem) {
        QAHooks::removeHooks();
        m_objects.clear();
    }
    QAService::instance()->initialize(QStringLiteral("ru.omprussia.qaservice.%1").arg(qApp->applicationFilePath().section(QLatin1Char('/'), -1)));
}

QQuickItem *QAEngine::findRootItem() const
{
    for (QObject *object : m_objects) {
        QQuickItem *root = findRootHelper(object);
        if (!root) {
            continue;
        }
        return root;
    }
    return nullptr;
}

QQuickItem *QAEngine::findRootHelper(QObject *object)
{
    QQuickView *view = qobject_cast<QQuickView*>(object);
    if (view) {
        return view->rootObject();
    }
    QQmlApplicationEngine *engine = qobject_cast<QQmlApplicationEngine*>(object);
    if (engine && !engine->rootObjects().isEmpty()) {
        QQuickWindow *window = qobject_cast<QQuickWindow *>(engine->rootObjects().first());
        if (window) {
            return window->contentItem();
        }
    }
    return nullptr;
}

void QAEngine::addObject(QObject *o)
{
    if (!m_rootItem && o != this) {
        m_objects.append(o);
    }
}

void QAEngine::removeObject(QObject *o)
{
    m_objects.removeAll(o);
}

QQuickItem *QAEngine::rootItem() const
{
    return m_rootItem;
}

QAEngine *QAEngine::instance()
{
    if (!s_instance) {
        QScopedValueRollback<quintptr> rb(qtHookData[QAHooks::AddQObject], reinterpret_cast<quintptr>(nullptr));
        s_instance = new QAEngine;
    }
    return s_instance;
}

QAEngine::QAEngine(QObject *parent)
    : QObject(parent)
{
}

QAEngine::~QAEngine()
{
}
