#ifndef QASERVICE_HPP
#define QASERVICE_HPP

#include <QObject>
#include <QtDBus>
#include <QDBusContext>

class QAAdaptor;
class QAService : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "ru.omprussia.qaservice")
public:
    bool initialize(const QString &serviceName);

    static QAService *instance();
    explicit QAService(QObject *parent = nullptr);

private slots:
    QString dumpTree();
    void doDumpTree(const QDBusMessage &message);

    QString findObjectByProperty(const QString &parentObject, const QString &property, const QString &value);
    void doFindObjectByProperty(const QString &parentObject, const QString &property, const QString &value, const QDBusMessage &message);

    bool sendMouseEvent(const QString &object, const QVariantMap &event);
    void doSendMouseEvent(const QString &object, const QVariantMap &event, const QDBusMessage &message);

private:
    void sendMessageReply(const QDBusMessage &message, const QVariant &result);

    QAAdaptor *m_adaptor = nullptr;
};

#endif // QASERVICE_HPP
