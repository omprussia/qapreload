#ifndef QASERVICE_HPP
#define QASERVICE_HPP

#include <QObject>
#include <QtDBus>
#include <QDBusContext>

class QQuickItem;
class QAAdaptor;
class QJsonObject;
class QAService : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "ru.omprussia.qaservice")
public:
    bool initialize(const QString &serviceName);

    static QAService *instance();
    static void sendMessageReply(const QDBusMessage &message, const QVariant &result);

private slots:
    QString dumpTree();
    QString dumpCurrentPage();
    QByteArray grabWindow();
    QByteArray grabCurrentPage();

    QStringList findObjectsByProperty(const QString &parentObject, const QString &property, const QString &value);
    QStringList findObjectsByClassname(const QString &parentObject, const QString &className);

    void clickPoint(int posx, int posy);
    void clickObject(const QString &object);

    void mouseSwipe(int startx, int starty, int stopx, int stopy);

private:
    explicit QAService(QObject *parent = nullptr);
    QAAdaptor *m_adaptor = nullptr;

    QHash<QString, QQuickItem*> m_objects;
};

#endif // QASERVICE_HPP
