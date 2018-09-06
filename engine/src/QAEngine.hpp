#ifndef QAENGINE_HPP
#define QAENGINE_HPP

#include <QHash>
#include <QObject>

class QQmlEngine;
class TestResult : public QObject
{
    Q_OBJECT
public:
    explicit TestResult(QObject* parent = nullptr);
    TestResult(const TestResult &other);

    Q_PROPERTY(bool success MEMBER success NOTIFY successChanged)
    bool success = true;
    Q_PROPERTY(QString message MEMBER message NOTIFY messageChanged)
    QString message;

public slots:
    void raise();

signals:
    void successChanged();
    void messageChanged();

private:
    QQmlEngine* m_engine;
};
Q_DECLARE_METATYPE(TestResult)

class QQuickItem;
class QDBusMessage;
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

    Q_PROPERTY(QQuickItem* rootItem READ rootItem CONSTANT)
    QQuickItem* rootItem();

    static QString getText(QQuickItem *item);
    static QString className(QQuickItem* item);
    static QPointF getAbsPosition(QQuickItem* item);

    static QQuickItem* findItemByObjectName(const QString &objectName, QQuickItem* parentItem = nullptr);
    static QVariantList findItemsByClassName(const QString &className, QQuickItem* parentItem = nullptr);
    static QVariantList findItemsByText(const QString &text, bool partial = true, QQuickItem* parentItem = nullptr);
    static QVariantList findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem* parentItem = nullptr);
    static QQuickItem* findParentFlickable(QQuickItem* rootItem = nullptr);
    static QVariantList findNestedFlickable(QQuickItem* parentItem = nullptr);

    static QVariant executeJS(const QString &jsCode, QQuickItem *item);

    static void print(const QString &text);

private slots:
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

    void loadSailfishTest(const QString &fileName, const QDBusMessage &message);
    void clearComponentCache();

    void setEventFilterEnabled(bool enable, const QDBusMessage &message);

    void onTouchEvent(const QTouchEvent &event);
    void onKeyEvent(QKeyEvent *event);

    void onChildrenChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    friend class SailfishTest;
    friend class LipstickTestHelper;

    QQmlEngine* getEngine();
    TestResult* getTestResult(const QString& functionName);

    QJsonObject recursiveDumpTree(QQuickItem *rootItem, int depth = 0);
    QJsonObject dumpObject(QQuickItem *item, int depth = 0);


    QQuickItem* getCurrentPage();

    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &property, const QString &value);
    QStringList recursiveFindObjects(QQuickItem *parentItem, const QString &className);

    void sendGrabbedObject(QQuickItem *item, const QDBusMessage &message);

    explicit QAEngine(QObject *parent = nullptr);

    QQuickItem *m_rootItem = nullptr;

    QAMouseEngine *m_mouseEngine = nullptr;
    QAKeyEngine *m_keyEngine = nullptr;

    QHash<QString, TestResult*> m_testResults;

};

#endif // QAENGINE_HPP
