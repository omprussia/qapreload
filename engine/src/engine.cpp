#include "QAEngine.hpp"

extern "C" void loadPlugin(QQuickItem *rootItem)
{
    QAEngine::instance()->initialize(rootItem);
}

extern "C" void rootReady()
{
    QAEngine::instance()->ready();
}
