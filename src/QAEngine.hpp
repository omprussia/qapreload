#ifndef QAENGINE_HPP
#define QAENGINE_HPP

#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"

#include <QObject>
#include <QHash>

class QDBusMessage;
class QAHooks;
class QQuickItem;
class QAEngine : public QObject
{
    Q_OBJECT
public:
    static QAEngine *instance();

    virtual ~QAEngine();

public slots:
    void dumpTree(const QDBusMessage &message);
    void dumpCurrentPage(const QDBusMessage &message);

    void clickPoint(int posx, int posy);
    void clickObject(const QString &object);

    void pressAndHold(int posx, int posy);

    void mouseSwipe(int startx, int starty, int stopx, int stopy);

    void grabWindow(const QDBusMessage &message);
    void grabCurrentPage(const QDBusMessage &message);

    void pressKeys(const QString &keys);
    void pressBackspace(int count);
    void pressEnter(int count);

private slots:
    void onMouseEvent(QMouseEvent *event);
    void onKeyEvent(QKeyEvent *event);

private:
    QJsonObject recursiveDumpTree(QQuickItem *rootItem, int depth = 0);
    QJsonObject dumpObject(QQuickItem *item, int depth = 0);

    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &property, const QString &value);
    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &className);

    void sendGrabbedObject(QQuickItem *item, const QDBusMessage &message);

    explicit QAEngine(QObject *parent = nullptr);
    friend class QAHooks;

    void initialize();

    QQuickItem *m_rootItem = nullptr;

    Qt::MouseButton m_mouseButton = Qt::NoButton;

    QAMouseEngine *m_mouseEngine;
    QAKeyEngine * m_keyEngine;
};

#endif // QAENGINE_HPP
