#include "QAEngine.hpp"
#include "QAHooks.hpp"
#include "QAService.hpp"

#include <stdio.h>

#include <QDebug>

#include <QCoreApplication>
#include <QScopedValueRollback>
#include <QTimer>

#include <QQmlApplicationEngine>
#include <QtQuick/QQuickView>

#include <QtQuick/QQuickItem>

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
    QTimer::singleShot(delay, qApp, [this](){
        for (QObject *object : m_objects) {
            QQuickView *view = qobject_cast<QQuickView*>(object);
            if (view) {
                m_rootItem = view->rootObject();
            }
            QQmlApplicationEngine *engine = qobject_cast<QQmlApplicationEngine*>(object);
            if (engine) {
                QQuickWindow *window = qobject_cast<QQuickWindow *>(engine->rootObjects().first());
                if (window) {
                    m_rootItem = window->contentItem();
                }
            }
            if (m_rootItem) {
                qDebug() << Q_FUNC_INFO << "Root item:" << m_rootItem;
                QAHooks::removeHooks();
                m_objects.clear();
                break;
            }
        }

        QAService::instance()->initialize(QStringLiteral("ru.omprussia.qaservice.%1").arg(qApp->applicationFilePath().section(QChar('/'), -1)));
    });
}

void QAEngine::addObject(QObject *o)
{
//    printf("QAEngine::addObject: %p\n", o);

    if (!m_rootItem && o != this) {
        m_objects.append(o);
    }
}

void QAEngine::removeObject(QObject *o)
{
//    printf("QAEngine::removeObject: %p\n", o);

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
//        auto hook = qtHookData[QAHooks::AddQObject];
//        qtHookData[QAHooks::AddQObject] = reinterpret_cast<quintptr>(nullptr);
        s_instance = new QAEngine;
//        printf("QAEngine::instance: %p\n", s_instance);
//        qtHookData[QAHooks::AddQObject] = hook;
    }
    return s_instance;
}

QAEngine::QAEngine(QObject *parent)
    : QObject(parent)
{
//    printf("QAEngine::QAEngine: %p\n", this);
}

QAEngine::~QAEngine()
{
//    printf("QAEngine::~QAEngine\n");
}
