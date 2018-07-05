#include "QAEngine.hpp"
#include "QAHooks.hpp"
#include "QAService.hpp"

#include <QDebug>

#include <QCoreApplication>
#include <QGuiApplication>

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

void QAEngine::initialize()
{
    QAHooks::removeStartupHook();

    QGuiApplication *gui = qobject_cast<QGuiApplication*>(qApp);
    if (!gui) {
        deleteLater();
        return;
    }

    setParent(qApp);
    QTimer::singleShot(0, this, [this](){
        QWindowList windows = qGuiApp->topLevelWindows();
        for (QWindow *window : windows) {
            QQuickWindow *qWindow = qobject_cast<QQuickWindow*>(window);
            if (qWindow && qWindow->contentItem()) {
                m_rootItem = qWindow->contentItem();
                QAService::instance()->initialize(m_rootItem);
                break;
            }
        }
    });
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

        QAService::sendMessageReply(message, arr);
    });
}

void QAEngine::onMouseEvent(QMouseEvent *event)
{
    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootItem->window());
    wp->deliverMouseEvent(event);

    if (event->type() == QEvent::MouseButtonRelease && wp->mouseGrabberItem) {
        wp->mouseGrabberItem = nullptr;
    }
}

QAEngine *QAEngine::instance()
{
    if (!s_instance) {
        s_instance = new QAEngine;
    }
    return s_instance;
}

QAEngine::QAEngine(QObject *parent)
    : QObject(parent)
    , m_mouseEngine(new QAMouseEngine(this))
    , m_keyEngine(new QAKeyEngine(this))
{
    connect(m_mouseEngine, &QAMouseEngine::triggered, this, &QAEngine::onMouseEvent);
    connect(m_keyEngine, &QAKeyEngine::triggered, this, &QAEngine::onKeyEvent);
}

QAEngine::~QAEngine()
{
}

void QAEngine::dumpTree(const QDBusMessage &message)
{
    QJsonObject tree = recursiveDumpTree(m_rootItem);
    QJsonDocument doc(tree);
    const QByteArray dump = doc.toJson(QJsonDocument::Indented);

    QAService::sendMessageReply(message, QString::fromUtf8(dump));
}

void QAEngine::dumpCurrentPage(const QDBusMessage &message)
{
    if (m_rootItem->childItems().isEmpty()) {
        return;
    }

    QQuickItem *pageStack = m_rootItem->childItems().first()->property("pageStack").value<QQuickItem*>();
    if (!pageStack) {
        qWarning() << Q_FUNC_INFO << "Cannot find PageStack!";
        QAService::sendMessageError(message, QStringLiteral("PageStack not found"));
        return;
    }

    QQuickItem *currentPage = pageStack->property("currentPage").value<QQuickItem*>();
    if (!currentPage) {
        qWarning() << Q_FUNC_INFO << "Cannot get currentPage from PageStack!";
        QAService::sendMessageError(message, QStringLiteral("currentPage not found"));
        return;
    }

    QJsonObject tree = recursiveDumpTree(currentPage);
    QJsonDocument doc(tree);
    const QByteArray dump = doc.toJson(QJsonDocument::Indented);

    QAService::sendMessageReply(message, QString::fromUtf8(dump));
}

void QAEngine::clickPoint(int posx, int posy)
{
    m_mouseEngine->click(QPointF(posx, posy));
}

void QAEngine::pressAndHold(int posx, int posy)
{
    m_mouseEngine->pressAndHold(QPointF(posx, posy));
}

void QAEngine::mouseSwipe(int startx, int starty, int stopx, int stopy)
{
    m_mouseEngine->move(QPointF(startx, starty), QPointF(stopx, stopy));
}

void QAEngine::grabWindow(const QDBusMessage &message)
{
    sendGrabbedObject(m_rootItem, message);
}

void QAEngine::grabCurrentPage(const QDBusMessage &message)
{
    QQuickItem *pageStack = m_rootItem->property("pageStack").value<QQuickItem*>();
    if (!pageStack) {
        qWarning() << Q_FUNC_INFO << "Cannot find PageStack!";
        QAService::sendMessageError(message, QStringLiteral("PageStack not found"));
        return;
    }

    QQuickItem *currentPage = pageStack->property("currentPage").value<QQuickItem*>();
    if (currentPage) {
        qWarning() << Q_FUNC_INFO << "Cannot find currentPage in PageStack!";
        QAService::sendMessageError(message, QStringLiteral("currentPage not found"));
        return;
    }

    sendGrabbedObject(currentPage, message);
}

void QAEngine::onKeyEvent(QKeyEvent *event)
{
    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootItem->window());
    wp->deliverKeyEvent(event);
}

void QAEngine::pressKeys(const QString &keys)
{
    m_keyEngine->pressKeys(keys);
}

void QAEngine::pressBackspace(int count)
{
    m_keyEngine->pressBackspace(count);
}

void QAEngine::pressEnter(int count)
{
    m_keyEngine->pressEnter(count);
}
