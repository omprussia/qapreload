#include <QCoreApplication>
#include <QTimer>
#include "QABridge.hpp"

#include <getdef.h>

int main(int argc, char *argv[])
{
    int uid_min = getdef_num("UID_MIN", -1);
    qputenv("XDG_RUNTIME_DIR", QStringLiteral("/run/user/%1").arg(uid_min).toUtf8());
    qputenv("DBUS_SESSION_BUS_ADDRESS", QStringLiteral("unix:path=/run/user/%1/dbus/user_bus_socket").arg(uid_min).toUtf8());

    QCoreApplication app(argc, argv);

    QABridge bridge;
    QTimer::singleShot(0, &bridge, &QABridge::start);
    return app.exec();
}
