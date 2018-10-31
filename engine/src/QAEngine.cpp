#include "QAEngine.hpp"
#include "QADBusService.hpp"
#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"
#include "QAPendingEvent.hpp"
#include "SailfishTest.hpp"

#include <QDebug>

#include <QCoreApplication>
#include <QGuiApplication>

#include <QMouseEvent>
#include <QKeyEvent>

#include <QTimer>

#include <QQmlApplicationEngine>
#include <QQmlExpression>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickView>

#include <QDBusMessage>

#include <private/qquickwindow_p.h>
#include <private/qquickitem_p.h>
#include "qpa/qwindowsysteminterface_p.h"

#include <private/qv4engine_p.h>

#include "SailfishTest.hpp"
#include "LipstickTestHelper.hpp"

static QAEngine *s_instance = nullptr;

bool QAEngine::isLoaded()
{
    return s_instance;
}

void QAEngine::initialize(QQuickItem *rootItem)
{
    setParent(qGuiApp);

    m_rootItem = rootItem;
}

void QAEngine::ready()
{
    m_mouseEngine = new QAMouseEngine(this);
    connect(m_mouseEngine, &QAMouseEngine::touchEvent, this, &QAEngine::onTouchEvent);

    m_keyEngine = new QAKeyEngine(this);
    connect(m_keyEngine, &QAKeyEngine::triggered, this, &QAEngine::onKeyEvent);

    QADBusService::instance()->initialize();

    if (QFileInfo::exists(QStringLiteral("/etc/qapreload-touch-indicator"))) {
        setTouchIndicator(true);
    }

    const QStringList args = qApp->arguments();
    const int testArgIndex = args.indexOf(QStringLiteral("--run-sailfish-test"));
    if (testArgIndex < 0) {
        return;
    }
    if (testArgIndex == args.length() - 1) {
        return;
    }
    const QString testName = args.at(testArgIndex + 1);
    loadSailfishTest(testName, QDBusMessage());
    qApp->quit();
}

QJsonObject QAEngine::recursiveDumpTree(QQuickItem *rootItem, int depth)
{
    QJsonObject object = dumpObject(rootItem, depth);

    QJsonArray childArray;

    QQuickItemPrivate *itemPrivate = QQuickItemPrivate::get(rootItem);

    int z = 0;
    for (QQuickItem *child : itemPrivate->paintOrderChildItems()) {
        QJsonObject childObject = recursiveDumpTree(child, ++z);
        childArray.append(QJsonValue(childObject));
    }

    object.insert(QStringLiteral("children"), QJsonValue(childArray));

    return object;
}

QJsonObject QAEngine::dumpObject(QQuickItem *item, int depth)
{
    QJsonObject object;

    const QString className = QAEngine::className(item);
    object.insert(QStringLiteral("classname"), QJsonValue(className));

    const QString id = QAEngine::uniqueId(item);
    object.insert(QStringLiteral("id"), QJsonValue(id));

    auto mo = item->metaObject();
    do {
      std::vector<std::pair<QString, QVariant> > v;
      v.reserve(mo->propertyCount() - mo->propertyOffset());
      for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i)
          v.emplace_back(mo->property(i).name(),
                         mo->property(i).read(item));
      std::sort(v.begin(), v.end());
      for (auto &i : v) {
          if (!object.contains(i.first)
                  && i.second.canConvert<QString>()) {
              object.insert(i.first, QJsonValue::fromVariant(i.second));
          }
      }
    } while ((mo = mo->superClass()));

    QRectF rectF(item->x(), item->y(), item->width(), item->height());
    QRect rect = rectF.toRect();
    object.insert(QStringLiteral("width"), QJsonValue(rect.width()));
    object.insert(QStringLiteral("height"), QJsonValue(rect.height()));
    object.insert(QStringLiteral("x"), QJsonValue(rect.x()));
    object.insert(QStringLiteral("y"), QJsonValue(rect.y()));
    object.insert(QStringLiteral("z"), QJsonValue(depth));

    QPointF position(item->x(), item->y());
    QPoint abs;
    if (item->parentItem()) {
        abs = m_rootItem->mapFromItem(item->parentItem(), position).toPoint();
    } else {
        abs = position.toPoint();
    }

    object.insert(QStringLiteral("abs_x"), QJsonValue(abs.x()));
    object.insert(QStringLiteral("abs_y"), QJsonValue(abs.y()));

    object.insert(QStringLiteral("objectName"), QJsonValue(item->objectName()));
    object.insert(QStringLiteral("enabled"), QJsonValue(item->isEnabled()));
    object.insert(QStringLiteral("visible"), QJsonValue(item->isVisible()));

    object.insert(QStringLiteral("mainTextProperty"), QAEngine::getText(item));

    return object;
}

QVariant QAEngine::executeJS(const QString &jsCode, QQuickItem *item)
{
    QQmlExpression expr(qmlEngine(item)->rootContext(), item, jsCode);
    bool isUndefined = false;
    const QVariant reply = expr.evaluate(&isUndefined);
    if (expr.hasError()) {
        qWarning() << Q_FUNC_INFO << expr.error().toString();
    }
    return isUndefined ? QVariant(QStringLiteral("undefined")) : reply.toString();
}

void QAEngine::print(const QString &text)
{
    QTextStream out(stderr, QIODevice::WriteOnly);
    out << text << endl;
    out.flush();
}

QQuickItem *QAEngine::getCurrentPage()
{
    QQuickItem *pageStack = getPageStack();
    if (!pageStack) {
        return nullptr;
    }

    QQuickItem *currentPage = pageStack->property("currentPage").value<QQuickItem*>();
    if (!currentPage) {
        qWarning() << Q_FUNC_INFO << "Cannot get currentPage from PageStack!";
        return nullptr;
    }

    return currentPage;
}

QQuickItem *QAEngine::getPageStack()
{
    QQuickItem *pageStack = QAEngine::instance()->rootItem()->property("pageStack").value<QQuickItem*>();
    if (!pageStack) {
        pageStack = QAEngine::instance()->rootItem()->childItems().first()->property("pageStack").value<QQuickItem*>();
        if (!pageStack) {
            qWarning() << Q_FUNC_INFO << "Cannot find PageStack!";
            return nullptr;
        }
    }
    return pageStack;
}

void QAEngine::sendGrabbedObject(QQuickItem *item, const QDBusMessage &message)
{
    QSharedPointer<QQuickItemGrabResult> grabber = item->grabToImage();
    connect(grabber.data(), &QQuickItemGrabResult::ready, [grabber, message]() {
        QByteArray arr;
        QBuffer buffer(&arr);
        buffer.open(QIODevice::WriteOnly);
        bool success = grabber->image().save(&buffer, "PNG");

        if (!success) {
            qWarning() << Q_FUNC_INFO << "Error saving image!";
        }

        if (arr.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "Saved empty image!";
        }

        QADBusService::sendMessageReply(message, arr);
    });
}

void QAEngine::onTouchEvent(const QTouchEvent &event)
{
    QWindowSystemInterface::handleTouchEvent(m_rootItem->window(), event.timestamp(), event.device(),
        QWindowSystemInterfacePrivate::toNativeTouchPoints(event.touchPoints(), m_rootItem->window()));
}

void QAEngine::onKeyEvent(QKeyEvent *event)
{
    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootItem->window());
    wp->deliverKeyEvent(event);
}

void QAEngine::onChildrenChanged()
{
    if (m_rootItem->childItems().isEmpty()) {
        return;
    }

    disconnect(m_rootItem, &QQuickItem::childrenChanged, this, &QAEngine::onChildrenChanged);

    QADBusService::instance()->initialize();
}

QString QAEngine::getText(QQuickItem *item)
{
    static const char *textProperties[] = {
        "label",
        "title",
        "description",
        "placeholderText",
        "text",
        "value",
        "name",
    };

    for (const char *textProperty : textProperties) {
        if (item->metaObject()->indexOfProperty(textProperty) > 0) {
            return item->property(textProperty).toString();
        }
    }

    return QString();
}

QString QAEngine::className(QQuickItem *item)
{
    return QString::fromLatin1(item->metaObject()->className()).section(QChar('_'), 0, 0);
}

QString QAEngine::uniqueId(QQuickItem *item)
{
    return QStringLiteral("%1_0x%2").arg(QAEngine::className(item))
            .arg(reinterpret_cast<quintptr>(item),
                 QT_POINTER_SIZE * 2, 16, QLatin1Char('0'));
}

QPointF QAEngine::getAbsPosition(QQuickItem *item)
{
    QPointF position(item->x(), item->y());
    QPoint abs;
    if (item->parentItem()) {
        abs = QAEngine::instance()->rootItem()->mapFromItem(item->parentItem(), position).toPoint();
    } else {
        abs = position.toPoint();
    }
    return abs;
}

QQuickItem *QAEngine::findItemByObjectName(const QString &objectName, QQuickItem *parentItem)
{
    if (!parentItem) {
        parentItem = QAEngine::instance()->rootItem();
    }

    QList<QQuickItem*> childItems = parentItem->childItems();
    for (QQuickItem *child : childItems) {
        if (child->objectName() == objectName) {
            return child;
        }
        QQuickItem *item = findItemByObjectName(objectName, child);
        if (item) {
            return item;
        }
    }
    return nullptr;
}

QVariantList QAEngine::findItemsByClassName(const QString &className, QQuickItem *parentItem)
{
    QVariantList items;

    if (!parentItem) {
        parentItem = QAEngine::instance()->rootItem();
    }

    QList<QQuickItem*> childItems = parentItem->childItems();
    for (QQuickItem *child : childItems) {
        if (QAEngine::className(child) == className) {
            items.append(QVariant::fromValue(child));
        }
        QVariantList recursiveItems = findItemsByClassName(className, child);
        items.append(recursiveItems);
    }
    return items;
}

QVariantList QAEngine::findItemsByText(const QString &text, bool partial, QQuickItem *parentItem)
{
    QVariantList items;

    if (!parentItem) {
        parentItem = QAEngine::instance()->rootItem();
    }

    QList<QQuickItem*> childItems = parentItem->childItems();
    for (QQuickItem *child : childItems) {
        const QString &itemText = QAEngine::getText(child);
        if ((partial && itemText.contains(text)) || (!partial && itemText == text)) {
            items.append(QVariant::fromValue(child));
        }
        QVariantList recursiveItems = findItemsByText(text, partial, child);
        items.append(recursiveItems);
    }
    return items;
}

QVariantList QAEngine::findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem)
{
    QVariantList items;

    if (!parentItem) {
        parentItem = QAEngine::instance()->rootItem();
    }

    QList<QQuickItem*> childItems = parentItem->childItems();
    for (QQuickItem *child : childItems) {
        if (child->property(propertyName.toLatin1().constData()) == propertyValue) {
            items.append(QVariant::fromValue(child));
        }
        QVariantList recursiveItems = findItemsByProperty(propertyName, propertyValue, child);
        items.append(recursiveItems);
    }
    return items;
}

QQuickItem *QAEngine::findParentFlickable(QQuickItem *rootItem)
{
    if (!rootItem) {
        return nullptr;
    }
    while (rootItem->parentItem()) {
        if (rootItem->parentItem()->metaObject()->indexOfProperty("flickableDirection") >= 0) {
            return rootItem->parentItem();
        }
        rootItem = rootItem->parentItem();
    }

    return nullptr;
}

QVariantList QAEngine::findNestedFlickable(QQuickItem *parentItem)
{
    QVariantList items;

    if (!parentItem) {
        parentItem = QAEngine::instance()->rootItem();
    }

    QList<QQuickItem*> childItems = parentItem->childItems();
    for (QQuickItem *child : childItems) {
        if (child->metaObject()->indexOfProperty("flickableDirection") >= 0) {
            items.append(QVariant::fromValue(child));
        }
        QVariantList recursiveItems = findNestedFlickable(child);
        items.append(recursiveItems);
    }
    return items;
}

QQuickItem *QAEngine::getApplicationWindow()
{
    return QAEngine::instance()->applicationWindow();
}

QAEngine *QAEngine::instance()
{
    if (!s_instance) {
        s_instance = new QAEngine;
        qRegisterMetaType<TestResult>();
    }
    return s_instance;
}

QAEngine::QAEngine(QObject *parent)
    : QObject(parent)
{
}

QAEngine::~QAEngine()
{
}

QQuickItem* QAEngine::rootItem()
{
    return m_rootItem;
}

QQuickItem *QAEngine::applicationWindow()
{
    if (!m_applicationWindow) {
        m_applicationWindow = QAEngine::instance()->rootItem();
        if (!qmlEngine(m_applicationWindow)) {
            m_applicationWindow = m_applicationWindow->childItems().first();
        }
    }
    return m_applicationWindow;
}

void QAEngine::dumpTree(const QDBusMessage &message)
{
    QJsonObject tree = recursiveDumpTree(QAEngine::instance()->rootItem());
    QJsonDocument doc(tree);
    const QByteArray dump = doc.toJson(QJsonDocument::Compact);

    QADBusService::sendMessageReply(message, QString::fromUtf8(dump));
}

void QAEngine::dumpCurrentPage(const QDBusMessage &message)
{
    if (m_rootItem->childItems().isEmpty()) {
        return;
    }

    QQuickItem *currentPage = QAEngine::getCurrentPage();
    if (!currentPage) {
        QADBusService::sendMessageError(message, QStringLiteral("currentPage not found"));
        return;
    }

    QJsonObject tree = recursiveDumpTree(currentPage);
    QJsonDocument doc(tree);
    const QByteArray dump = doc.toJson(QJsonDocument::Compact);

    QADBusService::sendMessageReply(message, QString::fromUtf8(dump));
}

void QAEngine::clickPoint(int posx, int posy, const QDBusMessage &message)
{
    connect(m_mouseEngine->click(QPointF(posx, posy)),
            &QAPendingEvent::completed, [message](){
        QADBusService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::pressAndHold(int posx, int posy, const QDBusMessage &message)
{
    connect(m_mouseEngine->pressAndHold(QPointF(posx, posy)),
            &QAPendingEvent::completed, [message](){
        QADBusService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::mouseMove(int startx, int starty, int stopx, int stopy, const QDBusMessage &message)
{
    connect(m_mouseEngine->move(QPointF(startx, starty), QPointF(stopx, stopy)),
            &QAPendingEvent::completed, [message](){
        QADBusService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::grabWindow(const QDBusMessage &message)
{
    sendGrabbedObject(QAEngine::getApplicationWindow(), message);
}

void QAEngine::grabCurrentPage(const QDBusMessage &message)
{
    QQuickItem *currentPage = QAEngine::getCurrentPage();
    if (!currentPage) {
        QADBusService::sendMessageError(message, QStringLiteral("currentPage not found"));
        return;
    }

    sendGrabbedObject(currentPage, message);
}

void QAEngine::pressEnter(int count, const QDBusMessage &message)
{
    connect(m_keyEngine->pressEnter(count),
            &QAPendingEvent::completed, [message](){
        QADBusService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::pressBackspace(int count, const QDBusMessage &message)
{
    connect(m_keyEngine->pressBackspace(count),
            &QAPendingEvent::completed, [message](){
        QADBusService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::pressKeys(const QString &keys, const QDBusMessage &message)
{
    connect(m_keyEngine->pressKeys(keys),
            &QAPendingEvent::completed, [message](){
        QADBusService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::clearFocus()
{
    if (!m_rootItem) {
        return;
    }

    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootItem->window());
    wp->clearFocusObject();
}

void QAEngine::executeInPage(const QString &jsCode, const QDBusMessage &message)
{
    QQuickItem *currentPage = QAEngine::getCurrentPage();
    if (!currentPage) {
        QADBusService::sendMessageError(message, QStringLiteral("currentPage not found"));
        return;
    }

    QADBusService::sendMessageReply(message, QAEngine::executeJS(jsCode, currentPage));
}

void QAEngine::executeInWindow(const QString &jsCode, const QDBusMessage &message)
{
    QQmlEngine *engine = qmlEngine(m_rootItem);
    QQuickItem *trueItem = m_rootItem;
    if (!engine) {
        trueItem = m_rootItem->childItems().first();
        engine = qmlEngine(trueItem);
        if (!engine) {
            QADBusService::sendMessageError(message, QStringLiteral("window engine not found"));
        }
    }

    QADBusService::sendMessageReply(message, QAEngine::executeJS(jsCode, trueItem));
}

void QAEngine::loadSailfishTest(const QString &fileName, const QDBusMessage &message)
{
    QQmlEngine *engine = qmlEngine(m_rootItem);
    QQuickItem *trueItem = m_rootItem;
    if (!engine) {
        trueItem = m_rootItem->childItems().first();
        engine = qmlEngine(trueItem);
    }
    if (!engine) {
        if (message.isDelayedReply()) {
            QADBusService::sendMessageError(message, QStringLiteral("window engine not found"));
        }
        return;
    }
    QQmlComponent component(engine, QUrl::fromLocalFile(fileName));
    if (!component.isReady()) {
        if (message.isDelayedReply()) {
            QADBusService::sendMessageError(message, component.errorString());
        }
        return;
    }
    QObject *object = component.create(qmlEngine(trueItem)->rootContext());
    if (!object) {
        if (message.isDelayedReply()) {
            QADBusService::sendMessageError(message, component.errorString());
        }
        return;
    }
    SailfishTest* test = qobject_cast<SailfishTest*>(object);
    if (!test) {
        qWarning() << Q_FUNC_INFO << fileName << "is not SailfishTest instance!";;
        object->deleteLater();
        return;
    }

    const QStringList objectFunctions = test->declarativeFunctions();

    if (objectFunctions.contains(QStringLiteral("init"))) {
        QAEngine::print(QStringLiteral("### CALLING init"));
        QMetaObject::invokeMethod(test, "init", Qt::DirectConnection);
    }

    QStringList testFunctions = objectFunctions.filter(QRegExp("test_\\d+"));
    std::sort(testFunctions.begin(), testFunctions.end(), [](const QString &t1, const QString &t2) {
        return t1.section(QChar('_'), 1).toInt() < t2.section(QChar('_'), 1).toInt();
    });

    int successCount = 0;
    int failCount = 0;

    QAEngine::print(QStringLiteral("### RUNNING TESTS"));
    for (const QString &testFunction : testFunctions) {
        QAEngine::print(QStringLiteral("## RUNNING TEST: %1").arg(testFunction));
        TestResult* tr = new TestResult(engine);
        m_testResults.insert(testFunction, tr);
        QMetaObject::invokeMethod(test,
                                  testFunction.toLatin1().constData(),
                                  Qt::DirectConnection);
        QAEngine::print(QStringLiteral("# TEST RESULT: %1").arg(tr->success ? QStringLiteral("Success") : QStringLiteral("Fail")));
        if (tr->success) {
            successCount++;
        } else {
            failCount++;
            QAEngine::print(QStringLiteral("# ERROR MESSAGE: %1").arg(tr->message));
        }
        m_testResults.remove(testFunction);
        tr->deleteLater();
    }

    QAEngine::print(QStringLiteral("### FINISHED TESTS"));
    QAEngine::print(QStringLiteral("# SUCCESS: %1").arg(successCount));
    QAEngine::print(QStringLiteral("# FAIL: %1").arg(failCount));

    object->deleteLater();
    engine->clearComponentCache();

    if (message.isDelayedReply()) {
        QADBusService::sendMessageReply(message, QString("done!"));
    }
}

void QAEngine::clearComponentCache()
{
    QQmlEngine *engine = getEngine();
    if (!engine) {
        return;
    }
    engine->clearComponentCache();
}

void QAEngine::setEventFilterEnabled(bool enable, const QDBusMessage &message)
{
    qWarning() << Q_FUNC_INFO << enable;
    if (enable) {
        qGuiApp->installEventFilter(this);
    } else {
        qGuiApp->removeEventFilter(this);
    }

    QADBusService::sendMessageReply(message, QVariantList());
}

void QAEngine::setTouchIndicatorEnabled(bool enable, const QDBusMessage &message)
{
    setTouchIndicator(enable);
    qWarning() << Q_FUNC_INFO << enable << m_touchFilter;
    QADBusService::sendMessageReply(message, QVariantList());
}

bool QAEngine::eventFilter(QObject *watched, QEvent *event)
{
    QQuickItem *item = qobject_cast<QQuickItem*>(watched);
    if (!item) {
        return QObject::eventFilter(watched, event);
    }
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
    {
        QTouchEvent *te = static_cast<QTouchEvent*>(event);
        qWarning() << "[TE]" << te->device() << te->window() << te->target();
        for (const QTouchEvent::TouchPoint &point : te->touchPoints()) {
            qWarning() << "[TP]" << point.pos() << point.rect() << point.pressure();
        }
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    {
        QInputEvent *ie = static_cast<QInputEvent*>(event);
        qWarning() << "[IE]" << ie->timestamp();
    }
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::FocusAboutToChange:
        qWarning() << "[EV]" << watched << event;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void QAEngine::setTouchIndicator(bool enable)
{
    qDebug() << Q_FUNC_INFO << enable;
    if (enable) {
        if (m_touchFilter) {
            return;
        }
        m_touchFilter = new TouchFilter(this);
    } else {
        if (!m_touchFilter) {
            return;
        }
        m_touchFilter->deleteLater();
        m_touchFilter = nullptr;
    }
}

QQmlEngine *QAEngine::getEngine()
{
    QQmlEngine *engine = qmlEngine(m_rootItem);
    if (!engine) {
        engine = qmlEngine(m_rootItem->childItems().first());
    }
    return engine;
}

TestResult *QAEngine::getTestResult(const QString &functionName)
{
    if (m_testResults.contains(functionName)) {
        return m_testResults.value(functionName);
    }
    return nullptr;
}

TestResult::TestResult(QObject *parent)
    : QObject(parent)
    , m_engine(qobject_cast<QQmlEngine*>(parent))
{

}

TestResult::TestResult(const TestResult &other)
    : QObject(other.parent())
{
    success = other.success;
    message = other.message;
}

void TestResult::raise()
{
    QV4::ExecutionEngine* eEngine = QV8Engine::getV4(m_engine);
    eEngine->throwError(message);
}

TouchFilter::TouchFilter(QObject *parent)
    : QObject(parent)
{
    QQmlEngine *engine = qmlEngine(QAEngine::instance()->rootItem());
    QQuickItem *trueItem = QAEngine::instance()->rootItem();
    if (!engine) {
        trueItem = QAEngine::instance()->rootItem()->childItems().first();
        engine = qmlEngine(trueItem);
    }
    if (!engine) {
        qWarning() << Q_FUNC_INFO << "Can't determine engine!";
        return;
    }
    engine->clearComponentCache();
    QQmlComponent component(engine, QUrl::fromLocalFile(QStringLiteral("/usr/share/qapreload/qml/TouchIndicator.qml")));
    if (!component.isReady()) {
        qWarning() << Q_FUNC_INFO << component.errorString();
        return;
    }
    QObject *object = component.create(qmlEngine(trueItem)->rootContext());
    if (!object) {
        qWarning() << Q_FUNC_INFO << component.errorString();
        return;
    }
    QQuickItem *item = qobject_cast<QQuickItem*>(object);
    if (!item) {
        qWarning() << Q_FUNC_INFO << object << "object is not QQuickitem!";
        return;
    }
    item->setParent(QAEngine::instance()->rootItem());
    item->setParentItem(QAEngine::instance()->rootItem());
    m_touchIndicator = item;
    qGuiApp->installEventFilter(this);
}

TouchFilter::~TouchFilter()
{
    qGuiApp->removeEventFilter(this);
    delete m_touchIndicator;
    m_touchIndicator = nullptr;
}

bool TouchFilter::eventFilter(QObject *watched, QEvent *event)
{
    QQuickItem *item = qobject_cast<QQuickItem*>(watched);
    if (!item) {
        return QObject::eventFilter(watched, event);
    }
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    {
        QTouchEvent *te = static_cast<QTouchEvent*>(event);
        if (te->touchPoints().isEmpty()) {
            break;
        }
        QMetaObject::invokeMethod(m_touchIndicator,
                                  "show",
                                  Qt::QueuedConnection,
                                  Q_ARG(QVariant, te->touchPoints().first().screenPos().toPoint()),
                                  Q_ARG(QVariant, te->touchPoints().first().rect().size().toSize()));
        break;
    }
    case QEvent::TouchEnd:
    case QEvent::TouchCancel:
    {
        QMetaObject::invokeMethod(m_touchIndicator,
                                  "hide",
                                  Qt::QueuedConnection);
    }
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}
