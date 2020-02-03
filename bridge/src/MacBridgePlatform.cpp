#include "MacBridgePlatform.hpp"
#include <QDebug>
#include <QProcess>

MacBridgePlatform::MacBridgePlatform(QObject *parent)
    : LinuxBridgePlatform(parent)
{

}

bool MacBridgePlatform::lauchAppStandalone(const QString &appName, const QStringList &arguments)
{
    qDebug()
        << Q_FUNC_INFO
        << appName << arguments;

    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("DYDL_INSERT_LIBRARIES", "/usr/local/lib/libqapreloadhook.dydl");
    process.setProcessEnvironment(env);
    process.setProgram(appName);
    process.setArguments(arguments);
    return process.startDetached();
}
