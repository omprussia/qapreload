#ifndef QADBUSSERVICE_HPP
#define QADBUSSERVICE_HPP

#include <QObject>
#include <QtDBus>
#include <QDBusContext>

class QABridgeInterface;
class QQuickItem;
class QAAdaptor;
class QJsonObject;
class QADBusService : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "ru.omprussia.qaservice")
public:

    static QADBusService *instance();
    static QString processName();

    static void sendMessageReply(const QDBusMessage &message, const QVariant &result);
    static void sendMessageError(const QDBusMessage &message, const QString &errorString);

public slots:
    void initialize();
    void deinitialize();

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
    QString loadSailfishTest(const QString &fileName);
    void clearComponentCache();

    void setEventFilterEnabled(bool enable);
    void setTouchIndicatorEnabled(bool enable);
    void hideTouchIndicator();

    int startSocket();

    void quit();

signals:
    void engineLoaded(bool loaded);

private:
    explicit QADBusService(QObject *parent = nullptr);
    QAAdaptor *m_adaptor = nullptr;
    QABridgeInterface *m_bridgeIface = nullptr;

    int m_loadCount = 0;

};

#endif // QADBUSSERVICE_HPP
