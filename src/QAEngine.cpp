#include "QAEngine.hpp"
#include "QAHooks.hpp"
#include "QAService.hpp"

#include <QDebug>

#include <QCoreApplication>
#include <QScopedValueRollback>
#include <QTimer>

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickView>

#include <QDBusMessage>

#include <private/qquickwindow_p.h>
#include <private/qquickitem_p.h>

static QAEngine *s_instance = nullptr;
static const char *c_initDelayValue = "QA_INSPECTOR_DELAY";

void QAEngine::initialize()
{
    setParent(qApp);
    int delay = 5000;
    if (Q_UNLIKELY(qEnvironmentVariableIsSet(c_initDelayValue))) {
        bool ok = false;
        const int newDelay = qEnvironmentVariableIntValue(c_initDelayValue, &ok);
        if (!ok || newDelay < 0) {
            qWarning("The delay environment variable is not valid");
        } else {
            delay = newDelay;
        }
    }
    QTimer::singleShot(delay, this, &QAEngine::postInit);
}

void QAEngine::postInit()
{
    m_rootItem = findRootItem();
    qDebug() << Q_FUNC_INFO << "Root item:" << m_rootItem;
    if (m_rootItem) {
        QAHooks::removeHooks();
        m_rawObjects.clear();
    }
    QAService::instance()->initialize(QStringLiteral("ru.omprussia.qaservice.%1").arg(qApp->applicationFilePath().section(QLatin1Char('/'), -1)));
}

QQuickItem *QAEngine::findRootItem() const
{
    for (QObject *object : m_rawObjects) {
        QQuickItem *root = findRootHelper(object);
        if (!root) {
            continue;
        }
        return root;
    }
    return nullptr;
}

QQuickItem *QAEngine::findRootHelper(QObject *object)
{
    QQuickView *view = qobject_cast<QQuickView*>(object);
    if (view) {
        return view->rootObject();
    }
    QQmlApplicationEngine *engine = qobject_cast<QQmlApplicationEngine*>(object);
    if (engine && !engine->rootObjects().isEmpty()) {
        QQuickWindow *window = qobject_cast<QQuickWindow*>(engine->rootObjects().first());
        if (window) {
            return window->contentItem();
        }
    }
    return nullptr;
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

    const QString className = QString::fromLatin1(item->metaObject()->className()).section(QChar('_'), 0, 0);

    const QString id = QStringLiteral("%1_0x%2").arg(className)
                       .arg(reinterpret_cast<quintptr>(item),
                       QT_POINTER_SIZE * 2, 16, QLatin1Char('0'));

    m_idToObject.insert(id, item);
    m_objectToId.insert(item, id);

    object.insert(QStringLiteral("id"), QJsonValue(id));
    object.insert(QStringLiteral("classname"), QJsonValue(className));

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

    object.insert(QStringLiteral("enabled"), QJsonValue(item->isEnabled()));
    object.insert(QStringLiteral("visible"), QJsonValue(item->isVisible()));

    object.insert(QStringLiteral("checkable"), QJsonValue::fromVariant(item->property("checkable")));
    object.insert(QStringLiteral("checked"), QJsonValue::fromVariant(item->property("checked")));

    object.insert(QStringLiteral("busy"), QJsonValue::fromVariant(item->property("busy")));

    object.insert(QStringLiteral("text"), QJsonValue::fromVariant(item->property("text")));
    object.insert(QStringLiteral("title"), QJsonValue::fromVariant(item->property("title")));
    object.insert(QStringLiteral("name"), QJsonValue::fromVariant(item->property("name")));
    object.insert(QStringLiteral("label"), QJsonValue::fromVariant(item->property("label")));
    object.insert(QStringLiteral("value"), QJsonValue::fromVariant(item->property("value")));
    object.insert(QStringLiteral("description"), QJsonValue::fromVariant(item->property("description")));
    object.insert(QStringLiteral("placeholderText"), QJsonValue::fromVariant(item->property("placeholderText")));

    object.insert(QStringLiteral("currentIndex"), QJsonValue::fromVariant(item->property("currentIndex")));

    return object;
}

QStringList QAEngine::recursiveFindObjects(QQuickItem *parentItem, const QString &property, const QString &value)
{
    QStringList results;

    if (parentItem->property(property.toLatin1().constData()).toString() == value
            && m_objectToId.contains(parentItem)) {

        results.append(m_objectToId[parentItem]);
    }

    for (QQuickItem *child : parentItem->childItems()) {
        QStringList objects = recursiveFindObjects(child, property, value);
        if (!objects.isEmpty()) {
            results.append(objects);
        }
    }

    return results;
}

QStringList QAEngine::recursiveFindObjects(QQuickItem *parentItem, const QString &className)
{
    QStringList results;

    if (QString::fromLatin1(parentItem->metaObject()->className()).startsWith(QStringLiteral("%1_").arg(className))) {
        results.append(m_objectToId[parentItem]);
    }

    for (QQuickItem *child : parentItem->childItems()) {
        QStringList objects = recursiveFindObjects(child, className);
        if (!objects.isEmpty()) {
            results.append(objects);
        }
    }

    return results;
}

void QAEngine::onMouseEvent(QMouseEvent *event)
{
    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootItem->window());
    wp->deliverMouseEvent(event);
}

void QAEngine::addObject(QObject *o)
{
    if (!m_rootItem) {
        m_rawObjects.append(o);
    }
}

void QAEngine::removeObject(QObject *o)
{
    m_rawObjects.removeAll(o);

    if (!m_rootItem || m_objectToId.isEmpty()) {
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem*>(o);
    if (!item) {
        return;
    }

    if (m_objectToId.contains(item)) {
        const QString &key = m_objectToId.take(item);
        m_idToObject.remove(key);
    }

//        QMutableHashIterator<QQuickItem*, QString> it(m_objectToId);
//        while (it.hasNext()) {
//            it.next();
//            if (it.key() == o) {
//                m_idToObject.remove(it.value());
//                it.remove();
//                return;
//            }
//        }
}

QAEngine *QAEngine::instance()
{
    if (!s_instance) {
        QScopedValueRollback<quintptr> rb(qtHookData[QAHooks::AddQObject], reinterpret_cast<quintptr>(nullptr));
        s_instance = new QAEngine;
    }
    return s_instance;
}

QAEngine::QAEngine(QObject *parent)
    : QObject(parent)
    , m_mouseEngine(new QAMouseEngine(this))
{
    connect(m_mouseEngine, &QAMouseEngine::triggered, this, &QAEngine::onMouseEvent);
}

QAEngine::~QAEngine()
{
}

void QAEngine::dumpTree(const QDBusMessage &message)
{
    m_idToObject.clear();
    m_objectToId.clear();

    QJsonObject tree = recursiveDumpTree(m_rootItem);
    QJsonDocument doc(tree);
    const QByteArray dump = doc.toJson(QJsonDocument::Indented);

    QAService::sendMessageReply(message, QString::fromUtf8(dump));
}

void QAEngine::dumpCurrentPage(const QDBusMessage &message)
{
    QString result;

    QQuickItem *pageStack = m_rootItem->property("pageStack").value<QQuickItem*>();
    if (pageStack) {
        qDebug() << Q_FUNC_INFO << pageStack;
        QQuickItem *currentPage = pageStack->property("currentPage").value<QQuickItem*>();
        if (currentPage) {
            qDebug() << Q_FUNC_INFO << currentPage;

            m_idToObject.clear();
            m_objectToId.clear();

            QJsonObject tree = recursiveDumpTree(currentPage);
            QJsonDocument doc(tree);
            const QByteArray dump = doc.toJson(QJsonDocument::Indented);
            result = QString::fromUtf8(dump);
        }
    }

    QAService::sendMessageReply(message, result);
}

void QAEngine::findObjectsByProperty(const QString &parentObject, const QString &property, const QString &value, const QDBusMessage &message)
{
    if (m_idToObject.contains(parentObject)) {
        recursiveDumpTree(m_rootItem);
    }

    QQuickItem *item = nullptr;
    if (parentObject.isEmpty()) {
        item = m_rootItem;
    } else if (m_idToObject.contains(parentObject)) {
        item = m_idToObject[parentObject];
    }
    qDebug() << Q_FUNC_INFO << item;

    QStringList result;
    if (item) {
        result = recursiveFindObjects(item, property, value);
    }

    QAService::sendMessageReply(message, result);
}

void QAEngine::findObjectsByClassname(const QString &parentObject, const QString &className, const QDBusMessage &message)
{
    if (m_idToObject.contains(parentObject)) {
        recursiveDumpTree(m_rootItem);
    }

    QQuickItem *item = nullptr;
    if (parentObject.isEmpty()) {
        item = m_rootItem;
    } else if (m_idToObject.contains(parentObject)) {
        item = m_idToObject[parentObject];
    }
    qDebug() << Q_FUNC_INFO << item;

    QStringList result;
    if (item) {
        result = recursiveFindObjects(item, className);
    }

    QAService::sendMessageReply(message, result);
}

void QAEngine::clickPoint(int posx, int posy)
{
    QPointF point(posx, posy);
    m_mouseEngine->click(point);
}

void QAEngine::clickObject(const QString &object)
{
    if (m_idToObject.contains(object)) {
        recursiveDumpTree(m_rootItem);
    }

    if (!m_idToObject.contains(object)) {
        qWarning() << Q_FUNC_INFO << "No such object!" << object;
        return;
    }

    QQuickItem *item = m_idToObject[object];

    QPointF position(item->x(), item->y());
    QPointF abs;
    if (item->parentItem()) {
        abs = m_rootItem->mapFromItem(item->parentItem(), position);
    } else {
        abs = position;
    }

    QPointF point(abs.x() + item->width() / 2, abs.y() + item->height() / 2);

    m_mouseEngine->click(point);
}

void QAEngine::mouseSwipe(int startx, int starty, int stopx, int stopy)
{
    m_mouseEngine->move(QPointF(startx, starty), QPointF(stopx, stopy));
}

void QAEngine::grabWindow(const QDBusMessage &message)
{
    qDebug() << Q_FUNC_INFO;

    QSharedPointer<QQuickItemGrabResult> grabber = m_rootItem->grabToImage();
    connect(grabber.data(), &QQuickItemGrabResult::ready, [grabber, message]() {
        QByteArray arr;
        QBuffer buffer(&arr);
        buffer.open(QIODevice::WriteOnly);
        grabber->image().save(&buffer, "PNG");

        QAService::sendMessageReply(message, arr);
    });
}

void QAEngine::grabCurrentPage(const QDBusMessage &message)
{
    qDebug() << Q_FUNC_INFO;

    QQuickItem *pageStack = m_rootItem->property("pageStack").value<QQuickItem*>();
    if (pageStack) {
        qDebug() << Q_FUNC_INFO << pageStack;
        QQuickItem *currentPage = pageStack->property("currentPage").value<QQuickItem*>();
        if (currentPage) {
            qDebug() << Q_FUNC_INFO << currentPage;
            QSharedPointer<QQuickItemGrabResult> grabber = currentPage->grabToImage();
            connect(grabber.data(), &QQuickItemGrabResult::ready, [grabber, message]() {
                QByteArray arr;
                QBuffer buffer(&arr);
                buffer.open(QIODevice::WriteOnly);
                grabber->image().save(&buffer, "PNG");

                QAService::sendMessageReply(message, arr);
                return;
            });
        }
    }

    QAService::sendMessageReply(message, QByteArray());
}
