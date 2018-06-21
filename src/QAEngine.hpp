#ifndef QAENGINE_HPP
#define QAENGINE_HPP

#include <QObject>

class QAHooks;
class QQuickItem;
class QAEngine : public QObject
{
    Q_OBJECT
public:
    QQuickItem *rootItem();

    static QAEngine *instance();

    virtual ~QAEngine();

private:
    explicit QAEngine(QObject *parent = nullptr);
    friend class QAHooks;
    void initialize();
    void addObject(QObject *o);
    void removeObject(QObject *o);

    QQuickItem *m_rootItem = nullptr;
    QList<QObject*> m_objects;

};

#endif // QAENGINE_HPP
