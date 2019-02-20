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

    explicit SailfishTest(QObject *parent = nullptr);

    QStringList declarativeFunctions() const;

public slots:
    QQuickItem *findItemByObjectName(const QString &objectName, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByClassName(const QString &className, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByText(const QString &text, bool partial = true, QQuickItem *parentItem = nullptr);
    QVariantList findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem = nullptr);

    QQuickItem *findParentFlickable(QQuickItem *rootItem = nullptr);
    QVariantList findNestedFlickable(QQuickItem *parentItem = nullptr);

    QVariantList openContextMenu(QQuickItem *item);
    QVariantList filterVisibleItems(const QVariantList &items);

    void clickContextMenuItem(QQuickItem *item, const QString &text, bool partial = true);
    void clickContextMenuItem(QQuickItem *item, int index);

    QPointF getAbsPosition(QQuickItem *item) const;

    void enterCode(const QString &code);

    QQuickItem *getCurrentPage();

    void pullDownTo(const QString &text);
    void pullDownTo(int index);

    void pushUpTo(const QString &text);
    void pushUpTo(int index);

    void scrollToItem(QQuickItem *item);

    void clickItem(QQuickItem *item);
    void pressAndHoldItem(QQuickItem *item, int delay = 800);

    void clickPoint(int posx, int posy);
    void pressAndHold(int posx, int posy, int delay = 800);
    void mouseSwipe(int startx, int starty, int stopx, int stopy);
    void mouseDrag(int startx, int starty, int stopx, int stopy, int delay = 1200);

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
    void waitForPageChange(int timeout = 1000);
    void waitForPropertyChange(QQuickItem *item, const QString &propertyName, const QVariant &value, int timeout = 10000);

    void assert(const QString &text);
    void message(const QString &text);

    void assertEqual(const QVariant &value1, const QVariant &value2, const QString &text = QString());
    void assertNotEqual(const QVariant &value1, const QVariant &value2, const QString &text = QString());
    void assertTrue(bool value, const QString &text = QString());
    void assertFalse(bool value, const QString &text = QString());

    bool compareEqual(const QVariant &value1, const QVariant &value2);
    bool compareNotEqual(const QVariant &value1, const QVariant &value2);
    bool compareTrue(bool value);
    bool compareFalse(bool value);

private slots:
    void onPropertyChanged();
};

#endif // SAILFISHTEST_HPP
