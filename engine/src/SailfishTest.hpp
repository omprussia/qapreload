#ifndef SAILFISHTEST_HPP
#define SAILFISHTEST_HPP

#include <QObject>
#include <QVariant>
#include <QPointF>

class QQuickItem;
class SailfishTest : public QObject
{
    Q_OBJECT
public:
    enum SwipeDirection {
        SwipeDirectionUp,
        SwipeDirectionLeft,
        SwipeDirectionRight,
        SwipeDirectionDown,
    };
    Q_ENUM(SwipeDirection)

    enum PeekDirection {
        PeekDirectionUp,
        PeekDirectionLeft,
        PeekDirectionRight,
        PeekDirectionDown,
    };
    Q_ENUM(PeekDirection)

    QStringList declarativeFunctions() const;

public slots:
    QQuickItem* findItemByObjectName(const QString &objectName, QQuickItem* parentItem = nullptr);
    QVariantList findItemsByClassName(const QString &className, QQuickItem* parentItem = nullptr);
    QVariantList findItemsByText(const QString &text, bool partial = true, QQuickItem* parentItem = nullptr);
    QVariantList findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem* parentItem = nullptr);
    QQuickItem* findParentFlickable(QQuickItem* rootItem = nullptr);
    QVariantList findNestedFlickable(QQuickItem* parentItem = nullptr);

    QPointF getAbsPosition(QQuickItem* item) const;

    void enterCode(const QString &code);

    QQuickItem* getCurrentPage();

    void pullDownTo(const QString &text);
    void pullDownTo(int index);

    void pushUpTo(const QString &text);
    void pushUpTo(int index);

    void scrollToItem(QQuickItem* item);

    void clickItem(QQuickItem *item);
    void pressAndHold(QQuickItem* item);

    void clickPoint(int posx, int posy);
    void pressAndHold(int posx, int posy);
    void mouseMove(int startx, int starty, int stopx, int stopy);

    void goBack();
    void goForward();

    void swipe(SwipeDirection direction);
    void peek(PeekDirection direction);

    void pressEnter(int count);
    void pressBackspace(int count);
    void pressKeys(const QString &keys);

    void clearFocus();

    void saveScreenshot(const QString &location, bool fillBackground = true);

    void sleep(int msecs);
};

#endif // SAILFISHTEST_HPP
