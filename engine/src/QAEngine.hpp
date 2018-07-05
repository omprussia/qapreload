#ifndef QAENGINE_HPP
#define QAENGINE_HPP

#include <QObject>

class QDBusMessage;
class QQuickItem;
class QMouseEvent;
class QKeyEvent;
class QAKeyEngine;
class QAMouseEngine;
class QAEngine : public QObject
{
    Q_OBJECT
public:
    static QAEngine *instance();
    static void initialize();
    static bool isLoaded();

    virtual ~QAEngine();

public slots:
    void dumpTree(const QDBusMessage &message);
    void dumpCurrentPage(const QDBusMessage &message);

    void clickPoint(int posx, int posy);
    void pressAndHold(int posx, int posy);
    void mouseSwipe(int startx, int starty, int stopx, int stopy);

    void grabWindow(const QDBusMessage &message);
    void grabCurrentPage(const QDBusMessage &message);

    void pressEnter(int count);
    void pressBackspace(int count);
    void pressKeys(const QString &keys);

private slots:
    void onMouseEvent(QMouseEvent *event);
    void onKeyEvent(QKeyEvent *event);
    void onLateInitialization();

    void onChildrenChanged();

private:
    QJsonObject recursiveDumpTree(QQuickItem *rootItem, int depth = 0);
    QJsonObject dumpObject(QQuickItem *item, int depth = 0);

    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &property, const QString &value);
    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &className);

    void sendGrabbedObject(QQuickItem *item, const QDBusMessage &message);

    void waitForChildrens();

    explicit QAEngine(QObject *parent = nullptr);

    QQuickItem *m_rootItem = nullptr;

    Qt::MouseButton m_mouseButton = Qt::NoButton;

    QAMouseEngine *m_mouseEngine = nullptr;
    QAKeyEngine *m_keyEngine = nullptr;

};

#endif // QAENGINE_HPP
