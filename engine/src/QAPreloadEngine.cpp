#include "QAPreloadEngine.hpp"
#include "QAEngine.hpp"

#include <QCoreApplication>
#include <QGuiApplication>

#include <QQuickWindow>
#include <QQuickItem>

#include <QTimer>

namespace {

QAPreloadEngine *s_instance = nullptr;

QWindowList s_windowList;
QQuickItem *s_rootItem;

bool s_ready = false;

}

QAPreloadEngine *QAPreloadEngine::instance()
{
    if (!s_instance) {
        s_instance = new QAPreloadEngine;
    }
    return s_instance;
}

void QAPreloadEngine::initialize()
{
    if (s_instance) {
        return;
    }

    QGuiApplication *gui = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
    if (!gui) {
        return;
    }

    QTimer::singleShot(0, qApp, [](){ QAPreloadEngine::instance()->lateInitialization(); });
}

bool QAPreloadEngine::isLoaded()
{
    return s_instance;
}

QList<QWindow *> QAPreloadEngine::windowList()
{
    return s_windowList;
}

QQuickItem *QAPreloadEngine::rootItem()
{
    return s_rootItem;
}

bool QAPreloadEngine::isReady()
{
    return s_ready;
}

void QAPreloadEngine::lateInitialization()
{
    setParent(qGuiApp);

    QWindow *window = qGuiApp->topLevelAt(QPoint(1, 1));
    if (!window) {
        QWindowList windows = qGuiApp->allWindows();
        if (windows.isEmpty()) {
            return;
        }
        window = windows.first();
    }
    QQuickWindow *qWindow = qobject_cast<QQuickWindow*>(window);
    if (!qWindow) {
        return;
    }
    s_rootItem = qWindow->contentItem();

    if (!s_rootItem) {
        return;
    }

    QAEngine::instance()->initialize(s_rootItem);

    if (s_rootItem->childItems().isEmpty()) { // probably declarative cache
        connect(s_rootItem, &QQuickItem::childrenChanged, this, &QAPreloadEngine::onChildrenChanged); // let's wait for loading
    } else {
        setReady();
    }
}

void QAPreloadEngine::onChildrenChanged()
{
    if (s_rootItem->childItems().isEmpty()) {
        return;
    }

    disconnect(s_rootItem, &QQuickItem::childrenChanged, this, &QAPreloadEngine::onChildrenChanged);

    setReady();
}

void QAPreloadEngine::setReady()
{
    s_ready = true;

    QAEngine::instance()->ready();
}

QAPreloadEngine::QAPreloadEngine()
    : QObject(nullptr)
{

}
