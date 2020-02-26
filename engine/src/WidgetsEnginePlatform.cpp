#include "WidgetsEnginePlatform.hpp"

#include <QDebug>

WidgetsEnginePlatform::WidgetsEnginePlatform(QObject *parent)
    : GenericEnginePlatform(parent)
{

}

void WidgetsEnginePlatform::initialize()
{
    qWarning()
        << Q_FUNC_INFO;

    emit ready();
}
