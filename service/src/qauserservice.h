#ifndef QAUSERSERVICE_H
#define QAUSERSERVICE_H

#include <QObject>

class QAUserService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "ru.omprussia.qaservice")
public:
    explicit QAUserService(QObject *parent = nullptr);

public slots:
    void start();

    Q_SCRIPTABLE void launchApp(const QString &appName, const QStringList &arguments);

private:
    void launcherAction(const QString &appName, const QStringList &arguments);
    void launchProcess(const QString &appName, const QStringList &arguments);
};

#endif // QAUSERSERVICE_H
