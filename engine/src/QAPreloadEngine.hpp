#ifndef PRELOADENGINE_HPP
#define PRELOADENGINE_HPP

#include <QObject>

class QWindow;
class QQuickItem;
class QAPreloadEngine : public QObject
{
    Q_OBJECT
public:
    static QAPreloadEngine *instance();
    static void initialize();
    static bool isLoaded();

    static QList<QWindow *> windowList();
    static QQuickItem *rootItem();

    static bool isReady();

private slots:
    void lateInitialization();
    void onChildrenChanged();

private:
    void setReady();

    explicit QAPreloadEngine();

};

#endif // PRELOADENGINE_HPP
