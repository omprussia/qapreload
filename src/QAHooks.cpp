#include <QObject>

#include "QAHooks.hpp"
#include "QAEngine.hpp"

void QAHooks::installAllHooks()
{
    installObjectAddedHook();
    installObjectRemovedHook();
    installStartupHook();
}

void QAHooks::installStartupHook()
{
    qtHookData[QAHooks::Startup] = reinterpret_cast<quintptr>(&QAHooks::applicationStarted);
}

void QAHooks::installObjectAddedHook()
{
    qtHookData[QAHooks::AddQObject] = reinterpret_cast<quintptr>(&QAHooks::objectAdded);
}

void QAHooks::installObjectRemovedHook()
{
    qtHookData[QAHooks::RemoveQObject] = reinterpret_cast<quintptr>(&QAHooks::objectRemoved);
}

void QAHooks::removeAllHooks()
{
    removeStartupHook();
    removeObjectAddedHook();
    removeObjectRemovedHook();
}

void QAHooks::removeStartupHook()
{
    qtHookData[QAHooks::Startup] = reinterpret_cast<quintptr>(nullptr);
}

void QAHooks::removeObjectAddedHook()
{
    qtHookData[QAHooks::AddQObject] = reinterpret_cast<quintptr>(nullptr);
}

void QAHooks::removeObjectRemovedHook()
{
    qtHookData[QAHooks::RemoveQObject] = reinterpret_cast<quintptr>(nullptr);
}

void QAHooks::applicationStarted()
{
    QAEngine::instance()->initialize();
}

void QAHooks::objectAdded(QObject *o)
{
    QAEngine::instance()->addObject(o);
}

void QAHooks::objectRemoved(QObject *o)
{
    QAEngine::instance()->removeObject(o);
}
