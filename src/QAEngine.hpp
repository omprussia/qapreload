#ifndef QAENGINE_HPP
#define QAENGINE_HPP

#include "QAMouseEngine.hpp"

#include <QObject>
#include <QHash>

class QDBusMessage;
class QAHooks;
class QQuickItem;
class QAEngine : public QObject
{
    Q_OBJECT
public:
    using RunMethod = void (*)(const QVariant &v);

    static QAEngine *instance();

    virtual ~QAEngine();

public slots:
    void dumpTree(const QDBusMessage &message);
    void dumpCurrentPage(const QDBusMessage &message);

    void findObjectsByProperty(const QString &parentObject, const QString &property, const QString &value, const QDBusMessage &message);
    void findObjectsByClassname(const QString &parentObject, const QString &className, const QDBusMessage &message);

    void clickPoint(int posx, int posy);
    void clickObject(const QString &object);

    void mouseSwipe(int startx, int starty, int stopx, int stopy);

    void grabWindow(const QDBusMessage &message);
    void grabCurrentPage(const QDBusMessage &message);

private slots:
    void postInit();
    QQuickItem *findRootItem() const;

    void onMouseEvent(QMouseEvent *event);

private:
    QJsonObject recursiveDumpTree(QQuickItem *rootItem, int depth = 0);
    QJsonObject dumpObject(QQuickItem *item, int depth = 0);

    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &property, const QString &value);
    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &className);

    void sendGrabbedObject(QQuickItem *item, const QDBusMessage &message);

    explicit QAEngine(QObject *parent = nullptr);
    friend class QAHooks;

    void initialize();
    void addObject(QObject *o);
    void removeObject(QObject *o);
    static QQuickItem *findRootHelper(QObject *object);

    QQuickItem *m_rootItem = nullptr;

    QHash<QString, QQuickItem*> m_idToObject;
    QHash<QQuickItem*, QString> m_objectToId;

    QList<QObject*> m_rawObjects;

    Qt::MouseButton m_mouseButton = Qt::NoButton;

    QAMouseEngine *m_mouseEngine;

};

#endif // QAENGINE_HPP
