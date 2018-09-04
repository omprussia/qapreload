#ifndef QAENGINE_HPP
#define QAENGINE_HPP

#include <QObject>

class QDBusMessage;
class QQuickItem;
class QMouseEvent;
class QTouchEvent;
class QKeyEvent;
class QAKeyEngine;
class QAMouseEngine;
class QAEngine : public QObject
{
    Q_OBJECT
public:
    static QAEngine *instance();
    static bool isLoaded();

    void initialize(QQuickItem *rootItem);
    void ready();

    virtual ~QAEngine();

public slots:
    void dumpTree(const QDBusMessage &message);
    void dumpCurrentPage(const QDBusMessage &message);

    void clickPoint(int posx, int posy, const QDBusMessage &message);
    void pressAndHold(int posx, int posy, const QDBusMessage &message);
    void mouseMove(int startx, int starty, int stopx, int stopy, const QDBusMessage &message);

    void grabWindow(const QDBusMessage &message);
    void grabCurrentPage(const QDBusMessage &message);

    void pressEnter(int count, const QDBusMessage &message);
    void pressBackspace(int count, const QDBusMessage &message);
    void pressKeys(const QString &keys, const QDBusMessage &message);

    void clearFocus();

    void executeInPage(const QString &jsCode, const QDBusMessage &message);
    void executeInWindow(const QString &jsCode, const QDBusMessage &message);

    void setEventFilterEnabled(bool enable, const QDBusMessage &message);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onTouchEvent(const QTouchEvent &event);
    void onKeyEvent(QKeyEvent *event);

    void onChildrenChanged();

private:
    QString getText(QQuickItem *item) const;

    QJsonObject recursiveDumpTree(QQuickItem *rootItem, int depth = 0);
    QJsonObject dumpObject(QQuickItem *item, int depth = 0);

    QVariant executeJson(const QString &jsCode, QQuickItem *item);

    QQuickItem *getCurrentPage();

    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &property, const QString &value);
    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &className);

    void sendGrabbedObject(QQuickItem *item, const QDBusMessage &message);

    explicit QAEngine(QObject *parent = nullptr);

    QQuickItem *m_rootItem = nullptr;

    Qt::MouseButton m_mouseButton = Qt::NoButton;

    QAMouseEngine *m_mouseEngine = nullptr;
    QAKeyEngine *m_keyEngine = nullptr;

};

#endif // QAENGINE_HPP
