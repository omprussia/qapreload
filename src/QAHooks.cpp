#include <QObject>

#include "QAHooks.hpp"
#include "QAEngine.hpp"

void QAHooks::installHooks()
{
    qtHookData[QAHooks::AddQObject] = reinterpret_cast<quintptr>(&QAHooks::objectAdded);
    qtHookData[QAHooks::RemoveQObject] = reinterpret_cast<quintptr>(&QAHooks::objectRemoved);
    qtHookData[QAHooks::Startup] = reinterpret_cast<quintptr>(&QAHooks::applicationStarted);
}

void QAHooks::removeHooks()
{
    qtHookData[QAHooks::AddQObject] = reinterpret_cast<quintptr>(nullptr);
    qtHookData[QAHooks::RemoveQObject] = reinterpret_cast<quintptr>(nullptr);
    qtHookData[QAHooks::Startup] = reinterpret_cast<quintptr>(nullptr);
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
