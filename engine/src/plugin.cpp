#include <QtQml/qqml.h>
#include <QtQml/QQmlExtensionPlugin>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlEngine>

#include "SailfishTest.hpp"

class SailfishTestPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "ru.omprussia.sailfishtest")
public:
    void registerTypes(const char *uri)
    {
        qmlRegisterType<SailfishTest>(uri, 1, 0, "SailfishTest");
    }
};

#include "plugin.moc"
