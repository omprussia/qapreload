// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAEngine.hpp"
#include "QuickEnginePlatform.hpp"
#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"

#include <QBuffer>
#include <QDebug>
#include <QGuiApplication>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QQmlExpression>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>
#include <QXmlQuery>
#include <QXmlStreamWriter>

#include <private/qquickwindow_p.h>
#include <private/qquickitem_p.h>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(categoryQuickEnginePlatform, "omp.qaengine.platform.quick", QtInfoMsg)

QList<QObject *> QuickEnginePlatform::childrenList(QObject *parentItem)
{
    QList<QObject*> result;

    QQuickItem *quick = qobject_cast<QQuickItem*>(parentItem);
    if (!quick) {
        return result;
    }

    QQuickItemPrivate *p = QQuickItemPrivate::get(quick);
    if (!p) {
        return result;
    }

    for (QQuickItem *w : p->paintOrderChildItems()) {
        result.append(w);
    }

    return result;
}

QQuickItem *QuickEnginePlatform::getItem(const QString &elementId)
{
    return qobject_cast<QQuickItem*>(getObject(elementId));
}

QuickEnginePlatform::QuickEnginePlatform(QWindow *window)
    : GenericEnginePlatform(window)
{
}

QQuickItem *QuickEnginePlatform::findParentFlickable(QQuickItem *rootItem)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << rootItem;

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

QVariantList QuickEnginePlatform::findNestedFlickable(QQuickItem *parentItem)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << parentItem;

    QVariantList items;

    if (!parentItem) {
        parentItem = m_rootQuickItem;
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

QQuickItem *QuickEnginePlatform::getApplicationWindow()
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO;

    QQuickItem *applicationWindow = m_rootQuickItem;
    if (!qmlEngine(applicationWindow)) {
        applicationWindow = applicationWindow->childItems().first();
    }
    return applicationWindow;
}

QPoint QuickEnginePlatform::getAbsPosition(QObject *item)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << item;

    QQuickItem *q = qobject_cast<QQuickItem*>(item);
    if (!q) {
        return QPoint();
    }
    QPoint position = QPointF(q->x(), q->y()).toPoint();
    QPoint abs;
    if (q->parentItem()) {
        abs = m_rootQuickItem->mapFromItem(q->parentItem(), position).toPoint();
    } else {
        abs = position;
    }
    return abs;
}

QPoint QuickEnginePlatform::getPosition(QObject *item)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << item;

    QQuickItem *q = qobject_cast<QQuickItem*>(item);
    if (!q) {
        return QPoint();
    }
    return q->position().toPoint();
}

QSize QuickEnginePlatform::getSize(QObject *item)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << item;

    QQuickItem *q = qobject_cast<QQuickItem*>(item);
    if (!q) {
        return QSize();
    }
    return QSize(q->width(), q->height());
}

bool QuickEnginePlatform::isItemEnabled(QObject *item)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << item;

    QQuickItem *q = qobject_cast<QQuickItem*>(item);
    if (!q) {
        return false;
    }
    return q->isEnabled();
}

bool QuickEnginePlatform::isItemVisible(QObject *item)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << item;

    QQuickItem *q = qobject_cast<QQuickItem*>(item);
    if (!q) {
        return false;
    }
    return q->isVisible();
}

QVariant QuickEnginePlatform::executeJS(const QString &jsCode, QQuickItem *item)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << jsCode << item;

    QQmlExpression expr(qmlEngine(item)->rootContext(), item, jsCode);
    bool isUndefined = false;
    const QVariant reply = expr.evaluate(&isUndefined);
    if (expr.hasError()) {
        qCWarning(categoryQuickEnginePlatform)
            << Q_FUNC_INFO
            << expr.error().toString();
    }
    return isUndefined ? QVariant(QStringLiteral("undefined")) : reply;
}

QObject *QuickEnginePlatform::getParent(QObject *item)
{
    if (!item) {
        return nullptr;
    }
    QQuickItem *q = qobject_cast<QQuickItem*>(item);
    if (!q) {
        return item->parent();
    }
    return q->parentItem();
}

void QuickEnginePlatform::initialize()
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO;

    if (!m_rootWindow) {
        qCDebug(categoryQuickEnginePlatform)
            << Q_FUNC_INFO
            << "No windows!";
        return;
    }    

    QQuickWindow *qWindow = qobject_cast<QQuickWindow*>(m_rootWindow);

    if (!qWindow) {
        qCWarning(categoryQuickEnginePlatform)
            << Q_FUNC_INFO
            << m_rootWindow << "is not QQuickWindow!";
        return;
    }
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << qWindow;

    m_rootQuickWindow = qWindow;
    m_rootQuickItem = qWindow->contentItem();
    m_rootObject = m_rootQuickItem;

    emit ready();
}

void QuickEnginePlatform::grabScreenshot(QTcpSocket *socket, QObject *item, bool fillBackground)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << socket << item << fillBackground;

    QQuickItem *q = qobject_cast<QQuickItem*>(item);
    if (!q) {
        return;
    }

    if (!q->window()->isVisible()) {
        socketReply(socket, QString());
        return;
    }

    QSharedPointer<QQuickItemGrabResult> grabber = q->grabToImage();

    connect(grabber.data(), &QQuickItemGrabResult::ready, [this, grabber, socket, fillBackground]() {
        QByteArray arr;
        QBuffer buffer(&arr);
        buffer.open(QIODevice::WriteOnly);
        if (fillBackground) {
            QPixmap pixmap(grabber->image().width(), grabber->image().height());
            QPainter painter(&pixmap);
            painter.fillRect(0, 0, pixmap.width(), pixmap.height(), Qt::black);
            painter.drawImage(0, 0, grabber->image());
            pixmap.save(&buffer, "PNG");
        } else {
            grabber->image().save(&buffer, "PNG");
        }
        socketReply(socket, arr.toBase64());
    });
}

void QuickEnginePlatform::pressAndHoldItem(QObject *qitem, int delay)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << qitem << delay;

    if (!qitem) {
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem*>(qitem);
    if (!item) {
        return;
    }

    const QPointF itemAbs = getAbsPosition(item);
    pressAndHold(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2, delay);
}

void QuickEnginePlatform::clearFocus()
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO;

    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootQuickWindow);
    wp->clearFocusObject();
}

void QuickEnginePlatform::clearComponentCache()
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO;

    QQmlEngine *engine = getEngine();
    if (!engine) {
        return;
    }
    engine->clearComponentCache();
}

QQmlEngine *QuickEnginePlatform::getEngine(QQuickItem *item)
{
    if (!item) {
        item = m_rootQuickItem;
    }
    QQmlEngine *engine = qmlEngine(item);
    if (!engine) {
        engine = qmlEngine(item->childItems().first());
    }
    return engine;
}

void QuickEnginePlatform::getPageSourceCommand(QTcpSocket *socket)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    // TODO

    socketReply(socket, QString());
}

void QuickEnginePlatform::onKeyEvent(QKeyEvent *event)
{
    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootQuickWindow);
    wp->deliverKeyEvent(event);
}

void QuickEnginePlatform::executeCommand_touch_pressAndHold(QTcpSocket *socket, double posx, double posy)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << socket << posx << posy;

    pressAndHold(posx, posy);
    socketReply(socket, QString());
}

void QuickEnginePlatform::executeCommand_touch_mouseSwipe(QTcpSocket *socket, double posx, double posy, double stopx, double stopy)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << socket << posx << posy << stopx << stopy;

    mouseMove(posx, posy, stopx, stopy);
    socketReply(socket, QString());
}

void QuickEnginePlatform::executeCommand_touch_mouseDrag(QTcpSocket *socket, double posx, double posy, double stopx, double stopy)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << socket << posx << posy << stopx << stopy;

    mouseDrag(posx, posy, stopx, stopy);
    socketReply(socket, QString());
}

void QuickEnginePlatform::executeCommand_app_method(QTcpSocket *socket, const QString &elementId, const QString &method, const QVariantList &params)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId << method << params;

    QQuickItem *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString());
        return;
    }

    QGenericArgument arguments[10] = { QGenericArgument() };
    for (int i = 0; i < params.length(); i++) {
        arguments[i] = Q_ARG(QVariant, params[i]);
    }

    QVariant reply;
    QMetaObject::invokeMethod(
        item,
        method.toLatin1().constData(),
        Qt::DirectConnection,
        Q_RETURN_ARG(QVariant, reply),
        arguments[0],
        arguments[1],
        arguments[2],
        arguments[3],
        arguments[4],
        arguments[5],
        arguments[6],
        arguments[7],
        arguments[8],
        arguments[9]);
    socketReply(socket, reply);
}

void QuickEnginePlatform::executeCommand_app_js(QTcpSocket *socket, const QString &elementId, const QString &jsCode)
{
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId << jsCode;

    QQuickItem *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString());
        return;
    }

    QVariant result = executeJS(jsCode, item);
    qCDebug(categoryQuickEnginePlatform)
        << Q_FUNC_INFO
        << result;
    socketReply(socket, result);
}
