#ifndef QAENGINE_HPP
#define QAENGINE_HPP

#include <QHash>
#include <QObject>

class QQmlEngine;
class QQuickItem;
class TestResult : public QObject
{
    Q_OBJECT
public:
    explicit TestResult(QObject *parent = nullptr);
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
    QQmlEngine *m_engine;
};
Q_DECLARE_METATYPE(TestResult)

#ifdef Q_OS_SAILFISH
class TouchFilter : public QObject
{
    Q_OBJECT
public:
    explicit TouchFilter(QObject *parent = nullptr);
public slots:
    void hideImmediately();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    QQuickItem *m_touchIndicator = nullptr;

};
#endif

class QDBusMessage;
class QMouseEvent;
class QTouchEvent;
class QKeyEvent;
class QAKeyEngine;
class QAMouseEngine;
class QXmlStreamWriter;
class QAEngine : public QObject
{
    Q_OBJECT
public:
    static QAEngine *instance();
    static bool isLoaded();

    void initialize(QQuickItem *rootItem);
    void ready();

    virtual ~QAEngine();

    Q_PROPERTY(QQuickItem *rootItem READ rootItem CONSTANT)
    QQuickItem *rootItem() const;
    QQuickItem *coverItem() const;

    static QVariantList rootItems();

    Q_PROPERTY(QQuickItem *applicationWindow READ applicationWindow CONSTANT)
    QQuickItem *applicationWindow() const;

    static QString processName();

    static QString getText(QQuickItem *item);
    static QString className(QObject *item);
    static QString uniqueId(QQuickItem *item);
    static QPointF getAbsPosition(QQuickItem *item);

    static QQuickItem *findItemByObjectName(const QString &objectName, QQuickItem *parentItem = nullptr);
    static QVariantList findItemsByClassName(const QString &className, QQuickItem *parentItem = nullptr);
    static QVariantList findItemsByText(const QString &text, bool partial = true, QQuickItem *parentItem = nullptr);
    static QVariantList findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem = nullptr);
    static QVariantList findItemsByXpath(const QString &xpath, QQuickItem *parentItem = nullptr);
    static QQuickItem *findParentFlickable(QQuickItem *rootItem = nullptr);
    static QVariantList findNestedFlickable(QQuickItem *parentItem = nullptr);

    static QQuickItem *getApplicationWindow();
    static QQuickItem *getCurrentPage();
    static QQuickItem *getPageStack();

    static QVariant executeJS(const QString &jsCode, QQuickItem *item);

    static void print(const QString &text);

private slots:
#ifdef USE_DBUS
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

    void executeInPage(const QString &jsCode, const QDBusMessage &message);
    void executeInWindow(const QString &jsCode, const QDBusMessage &message);

    void loadSailfishTest(const QString &fileName, const QDBusMessage &message);

    void setEventFilterEnabled(bool enable, const QDBusMessage &message);
    void setTouchIndicatorEnabled(bool enable, const QDBusMessage &message);
    void hideTouchIndicator(const QDBusMessage &message);
#endif
    void hideTouchIndicator();
    void clearFocus();
    void clearComponentCache();

    void onTouchEvent(const QTouchEvent &event);
    void onMouseEvent(const QMouseEvent &event);
    void onKeyEvent(QKeyEvent *event);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    friend class SailfishTest;
    friend class LipstickTestHelper;
    friend class QASocketService;

#ifdef Q_OS_SAILFISH
    friend class TouchFilter;
    void setTouchIndicator(bool enable);

    TouchFilter *m_touchFilter = nullptr;
#endif
    QQmlEngine *getEngine();
    TestResult *getTestResult(const QString &functionName);

    void recursiveDumpXml(QXmlStreamWriter *writer, QQuickItem *rootItem, int depth = 0);
    QJsonObject recursiveDumpTree(QQuickItem *rootItem, int depth = 0) const;
    QJsonObject dumpObject(QQuickItem *item, int depth = 0) const;

#ifdef USE_DBUS
    void sendGrabbedObject(QQuickItem *item, const QDBusMessage &message);
#endif

    explicit QAEngine(QObject *parent = nullptr);

    QQuickItem *m_applicationWindow = nullptr;
    QQuickItem *m_rootItem = nullptr;

    QAMouseEngine *m_mouseEngine = nullptr;
    QAKeyEngine *m_keyEngine = nullptr;

    QHash<QString, TestResult*> m_testResults;

    QHash<QString, QStringList> m_blacklistedProperties;
};

#endif // QAENGINE_HPP
