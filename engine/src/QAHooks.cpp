#include <QObject>

#include "QAHooks.hpp"
#include "QAEngine.hpp"

void QAHooks::installStartupHook()
{
    qtHookData[QAHooks::Startup] = reinterpret_cast<quintptr>(&QAHooks::applicationStarted);
}

void QAHooks::removeStartupHook()
{
    qtHookData[QAHooks::Startup] = reinterpret_cast<quintptr>(nullptr);
}

void QAHooks::applicationStarted()
{
    QAEngine::instance()->initialize();
}
