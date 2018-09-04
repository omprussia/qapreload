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

    static QAService *instance();

    static void sendMessageReply(const QDBusMessage &message, const QVariant &result);
    static void sendMessageError(const QDBusMessage &message, const QString &errorString);

public slots:
    void initialize();

private slots:
    QString dumpTree();
    QString dumpCurrentPage();
    QByteArray grabWindow();
    QByteArray grabCurrentPage();

    void clickPoint(int posx, int posy);
    void pressAndHold(int posx, int posy);
    void mouseMove(int startx, int starty, int stopx, int stopy);

    void pressEnter(int count);
    void pressBackspace(int count);
    void pressKeys(const QString &keys);

    void clearFocus();
    QString executeInPage(const QString &jsCode);
    QString executeInWindow(const QString &jsCode);

    void setEventFilterEnabled(bool enable);

    void quit();

private:
    explicit QAService(QObject *parent = nullptr);
    QAAdaptor *m_adaptor = nullptr;

    int m_loadCount = 0;

};

#endif // QASERVICE_HPP
