#include "QAEngine.hpp"
#include "QuickEnginePlatform.hpp"
#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"

#include <QBuffer>
#include <QDebug>
#include <QGuiApplication>
#include <QIODevice>
#include <QJsonObject>
#include <QPainter>
#include <QQmlExpression>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>

#include <private/qquickwindow_p.h>
#include <private/qquickitem_p.h>

QuickEnginePlatform::QuickEnginePlatform(QObject *parent)
    : GenericEnginePlatform(parent)
{
}

QQuickItem *QuickEnginePlatform::findItemByObjectName(const QString &objectName, QQuickItem *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << objectName << parentItem;

    if (!parentItem) {
        parentItem = m_rootQuickItem;
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

QVariantList QuickEnginePlatform::findItemsByClassName(const QString &className, QQuickItem *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << className << parentItem;

    QVariantList items;

    if (!parentItem) {
        parentItem = m_rootQuickItem;
    }

    QList<QQuickItem*> childItems = parentItem->childItems();
    for (QQuickItem *child : childItems) {
        if (getClassName(child) == className) {
            items.append(QVariant::fromValue(child));
        }
        QVariantList recursiveItems = findItemsByClassName(className, child);
        items.append(recursiveItems);
    }
    return items;
}

QVariantList QuickEnginePlatform::findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << propertyName << propertyValue << parentItem;

    QVariantList items;

    if (!parentItem) {
        parentItem = m_rootQuickItem;
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

QVariantList QuickEnginePlatform::findItemsByText(const QString &text, bool partial, QQuickItem *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << text << partial << parentItem;

    QVariantList items;

    if (!parentItem) {
        parentItem = m_rootQuickItem;
    }

    QList<QQuickItem*> childItems = parentItem->childItems();
    for (QQuickItem *child : childItems) {
        const QString &itemText = getText(child);
        if ((partial && itemText.contains(text)) || (!partial && itemText == text)) {
            items.append(QVariant::fromValue(child));
        }
        QVariantList recursiveItems = findItemsByText(text, partial, child);
        items.append(recursiveItems);
    }
    return items;
}

void QuickEnginePlatform::findByProperty(QTcpSocket *socket, const QString &propertyName, const QVariant &propertyValue, bool multiple, QQuickItem *parentItem)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << propertyName << propertyValue << multiple << parentItem;

    QVariantList items = findItemsByProperty(propertyName, propertyValue, parentItem);
    elementReply(socket, items, multiple);
}

QQuickItem *QuickEnginePlatform::findParentFlickable(QQuickItem *rootItem)
{
    qDebug()
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
    qDebug()
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

QVariantList QuickEnginePlatform::filterVisibleItems(const QVariantList &items)
{
    qWarning()
        << Q_FUNC_INFO
        << items;

    QVariantList result;
    for (const QVariant &itemVariant : items) {
        QQuickItem *item = itemVariant.value<QQuickItem*>();
        if (!item || !item->isVisible()) {
            continue;
        }
        result.append(itemVariant);
    }
    return result;
}

QQuickItem *QuickEnginePlatform::getApplicationWindow()
{
    qWarning()
        << Q_FUNC_INFO;

    QQuickItem *applicationWindow = m_rootQuickItem;
    if (!qmlEngine(applicationWindow)) {
        applicationWindow = applicationWindow->childItems().first();
    }
    return applicationWindow;
}

QPointF QuickEnginePlatform::getAbsPosition(QQuickItem *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    QPointF position(item->x(), item->y());
    QPoint abs;
    if (item->parentItem()) {
        abs = m_rootQuickItem->mapFromItem(item->parentItem(), position).toPoint();
    } else {
        abs = position.toPoint();
    }
    return abs;
}

QString QuickEnginePlatform::getText(QQuickItem *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

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

QVariant QuickEnginePlatform::executeJS(const QString &jsCode, QQuickItem *item)
{
    qDebug()
        << Q_FUNC_INFO
        << jsCode << item;

    QQmlExpression expr(qmlEngine(item)->rootContext(), item, jsCode);
    bool isUndefined = false;
    const QVariant reply = expr.evaluate(&isUndefined);
    if (expr.hasError()) {
        qWarning() << Q_FUNC_INFO << expr.error().toString();
    }
    return isUndefined ? QVariant(QStringLiteral("undefined")) : reply;
}

void QuickEnginePlatform::initialize()
{
    qDebug()
        << Q_FUNC_INFO;

    QWindow *window = qGuiApp->topLevelWindows().first();
    if (!window) {
        qWarning()
            << Q_FUNC_INFO
            << "No windows!";
        return;
    }
    qDebug()
        << Q_FUNC_INFO
        << window;

    QQuickWindow *qWindow = qobject_cast<QQuickWindow*>(window);

    if (!qWindow) {
        qWarning()
            << Q_FUNC_INFO
            << window << "is not QQuickWindow!";
        return;
    }
    m_rootQuickWindow = qWindow;
    m_rootWindow = window;
    qDebug()
        << Q_FUNC_INFO
        << qWindow;
    m_rootQuickItem = qWindow->contentItem();
    if (!m_rootQuickItem) {
        qWarning()
            << Q_FUNC_INFO
            << "No root item!";
        return;
    }
    m_rootObject = m_rootQuickItem;

    if (m_rootQuickItem->childItems().isEmpty()) {
        qWarning()
            << Q_FUNC_INFO
            << "No children items!";
        return;
    }

    emit ready();
}

void QuickEnginePlatform::activateAppCommand(QTcpSocket *socket, const QString &appName)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << appName;

    if (appName != QAEngine::processName()) {
        qWarning()
            << Q_FUNC_INFO
            << appName << "is not" << QAEngine::processName();
        socketReply(socket, QString(), 1);
        return;
    }

    if (!m_rootQuickWindow) {
        qWarning()
            << Q_FUNC_INFO
            << "No window!";
        return;
    }

    m_rootQuickWindow->show();
    m_rootQuickWindow->raise();

    socketReply(socket, QString());
}

void QuickEnginePlatform::closeAppCommand(QTcpSocket *socket, const QString &appName)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << appName;

    if (appName != QAEngine::processName()) {
        qWarning()
            << Q_FUNC_INFO
            << appName << "is not" << QAEngine::processName();
        socketReply(socket, QString(), 1);
        return;
    }

    if (!m_rootQuickWindow) {
        qWarning()
            << Q_FUNC_INFO
            << "No window!";
        return;
    }

    socketReply(socket, QString());
    qApp->quit();
}

void QuickEnginePlatform::queryAppStateCommand(QTcpSocket *socket, const QString &appName)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << appName;

    if (appName != QAEngine::processName()) {
        qWarning()
            << Q_FUNC_INFO
            << appName << "is not" << QAEngine::processName();
        socketReply(socket, QString(), 1);
        return;
    }

    if (!m_rootQuickWindow) {
        qWarning()
            << Q_FUNC_INFO
            << "No window!";
        return;
    }

    const bool isAppActive = m_rootQuickWindow->isActive();
    socketReply(socket, isAppActive ? QStringLiteral("RUNNING_IN_FOREGROUND") : QStringLiteral("RUNNING_IN_BACKGROUND"));
}

void QuickEnginePlatform::backgroundCommand(QTcpSocket *socket, double seconds)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << seconds;

    if (!m_rootQuickWindow) {
        qWarning()
            << Q_FUNC_INFO
            << "No window!";
        return;
    }

    const int msecs = seconds * 1000;

    m_rootQuickWindow->lower();
    if (msecs > 0) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(msecs);
        loop.exec();
        m_rootQuickWindow->raise();
    }

    socketReply(socket, QString());
}

void QuickEnginePlatform::findElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector;

    findElement(socket, strategy, selector);
}

void QuickEnginePlatform::findElementsCommand(QTcpSocket *socket, const QString &strategy, const QString &selector)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector;

    findElement(socket, strategy, selector, true);
}

void QuickEnginePlatform::findElementFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector << elementId;

    QQuickItem *item = getItem(elementId);
    findElement(socket, strategy, selector, false, item);
}

void QuickEnginePlatform::findElementsFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector << elementId;

    QQuickItem *item = getItem(elementId);
    findElement(socket, strategy, selector, true, item);
}

void QuickEnginePlatform::findElement(QTcpSocket *socket, const QString &strategy, const QString &selector, bool multiple, QQuickItem *item)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector << multiple << item;

    QString fixStrategy = strategy;
    fixStrategy = fixStrategy.remove(QChar(u' '));
    const QString methodName = QStringLiteral("findStrategy_%1").arg(fixStrategy);
    if (!QAEngine::metaInvoke(socket, this, methodName, {selector, multiple, QVariant::fromValue(reinterpret_cast<QQuickItem*>(item))})) {
        findByProperty(socket, fixStrategy, selector, multiple, item);
    }
}

void QuickEnginePlatform::grabScreenshot(QTcpSocket *socket, QQuickItem *item, bool fillBackground)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << item << fillBackground;

    if (!item->window()->isVisible()) {
        socketReply(socket, QString());
        return;
    }

    QSharedPointer<QQuickItemGrabResult> grabber = item->grabToImage();

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

void QuickEnginePlatform::setProperty(QTcpSocket *socket, const QString &property, const QString &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << property << value << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        item->setProperty(property.toLatin1().constData(), value);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QuickEnginePlatform::clickItem(QQuickItem *item)
{
    const QPointF itemAbs = getAbsPosition(item);
    qWarning()
        << Q_FUNC_INFO
        << item << itemAbs;

    clickPoint(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2);
}

void QuickEnginePlatform::pressAndHoldItem(QQuickItem *item, int delay)
{
    qWarning()
        << Q_FUNC_INFO
        << item << delay;

    if (!item) {
        return;
    }
    const QPointF itemAbs = getAbsPosition(item);
    pressAndHold(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2, delay);
}

void QuickEnginePlatform::waitForPropertyChange(QQuickItem *item, const QString &propertyName, const QVariant &value, int timeout)
{
    qWarning()
        << Q_FUNC_INFO
        << item << propertyName << value << timeout;

    if (!item) {
        qWarning() << "item is null" << item;
        return;
    }
    int propertyIndex = item->metaObject()->indexOfProperty(propertyName.toLatin1().constData());
    if (propertyIndex < 0) {
        qWarning() << Q_FUNC_INFO << item << "property" << propertyName << "is not valid!";
        return;
    }
    const QMetaProperty prop = item->metaObject()->property(propertyIndex);
    if (prop.read(item) == value) {
        return;
    }
    if (!prop.hasNotifySignal()) {
        qWarning()
            << Q_FUNC_INFO
            << item << "property" << propertyName << "have on notifySignal!";
        return;
    }
    QEventLoop loop;
    QTimer timer;
    item->setProperty("WaitForPropertyChangeEventLoop", QVariant::fromValue(&loop));
    item->setProperty("WaitForPropertyChangePropertyName", propertyName);
    item->setProperty("WaitForPropertyChangePropertyValue", value);
    const QMetaMethod propertyChanged = metaObject()->method(metaObject()->indexOfSlot("onPropertyChanged()"));
    connect(item, prop.notifySignal(), this, propertyChanged);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeout);
    loop.exec();
    disconnect(item, prop.notifySignal(), this, propertyChanged);
}

QQuickItem *QuickEnginePlatform::getItem(const QString &elementId)
{
    QQuickItem *item = nullptr;
    if (m_items.contains(elementId)) {
        item = qobject_cast<QQuickItem*>(m_items.value(elementId));
    }
    qWarning()
        << Q_FUNC_INFO
        << elementId << item;

    return item;
}

void QuickEnginePlatform::getLocationCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        QJsonObject reply;
        const QPointF absPoint = getAbsPosition(item);
        reply.insert(QStringLiteral("centerx"), absPoint.x() + item->width() / 2);
        reply.insert(QStringLiteral("centery"), absPoint.y() + item->height() / 2);
        reply.insert(QStringLiteral("x"), absPoint.x());
        reply.insert(QStringLiteral("y"), absPoint.y());
        reply.insert(QStringLiteral("width"),item->width());
        reply.insert(QStringLiteral("height"), item->height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void QuickEnginePlatform::getLocationInViewCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        QJsonObject reply;
        reply.insert(QStringLiteral("centerx"), item->x() + item->width() / 2);
        reply.insert(QStringLiteral("centery"), item->y() + item->height() / 2);
        reply.insert(QStringLiteral("x"), item->x());
        reply.insert(QStringLiteral("y"), item->y());
        reply.insert(QStringLiteral("width"),item->width());
        reply.insert(QStringLiteral("height"), item->height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void QuickEnginePlatform::getAttributeCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << attribute << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        const QVariant reply = item->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void QuickEnginePlatform::getPropertyCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << attribute << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        const QVariant reply = item->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void QuickEnginePlatform::getTextCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        socketReply(socket, getText(item));
    } else {
        socketReply(socket, QString());
    }
}

void QuickEnginePlatform::getElementScreenshotCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        grabScreenshot(socket, item, true);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QuickEnginePlatform::getScreenshotCommand(QTcpSocket *socket)
{
    qWarning()
        << Q_FUNC_INFO
        << socket;

    grabScreenshot(socket, m_rootQuickItem, true);
}

void QuickEnginePlatform::elementEnabledCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("enabled"), elementId);
}

void QuickEnginePlatform::elementDisplayedCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("visible"), elementId);
}

void QuickEnginePlatform::elementSelectedCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("checked"), elementId);
}

void QuickEnginePlatform::getSizeCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        QJsonObject reply;
        reply.insert(QStringLiteral("width"), item->width());
        reply.insert(QStringLiteral("height"), item->height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QuickEnginePlatform::setValueImmediateCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    QStringList text;
    for (const QVariant &val : value) {
        text.append(val.toString());
    }

    setProperty(socket, QStringLiteral("text"), text.join(QString()), elementId);
}

void QuickEnginePlatform::replaceValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    setValueImmediateCommand(socket, value, elementId);
}

void QuickEnginePlatform::setValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    setValueImmediateCommand(socket, value, elementId);
}

void QuickEnginePlatform::clickCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        clickItem(item);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QuickEnginePlatform::clearCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    setProperty(socket, QStringLiteral("text"), QString(), elementId);
}

void QuickEnginePlatform::submitCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    m_keyEngine->pressEnter(1);
    socketReply(socket, QString());
}

void QuickEnginePlatform::getPageSourceCommand(QTcpSocket *socket)
{
    qWarning()
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

void QuickEnginePlatform::executeCommand_app_waitForPropertyChange(QTcpSocket *socket, const QString &elementId, const QString &propertyName, const QVariant &value, double timeout)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId << propertyName << value << timeout;

    QQuickItem *item = getItem(elementId);
    if (item) {
        waitForPropertyChange(item, propertyName, value, timeout);
    }
    socketReply(socket, QString());
}

void QuickEnginePlatform::executeCommand_touch_pressAndHold(QTcpSocket *socket, double posx, double posy)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << posx << posy;

    pressAndHold(posx, posy);
    socketReply(socket, QString());
}

void QuickEnginePlatform::executeCommand_touch_mouseSwipe(QTcpSocket *socket, double posx, double posy, double stopx, double stopy)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << posx << posy << stopx << stopy;

    mouseMove(posx, posy, stopx, stopy);
    socketReply(socket, QString());
}

void QuickEnginePlatform::executeCommand_touch_mouseDrag(QTcpSocket *socket, double posx, double posy, double stopx, double stopy)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << posx << posy << stopx << stopy;

    mouseDrag(posx, posy, stopx, stopy);
    socketReply(socket, QString());
}

void QuickEnginePlatform::executeCommand_app_method(QTcpSocket *socket, const QString &elementId, const QString &method, const QVariantList &params)
{
    qWarning()
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
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId << jsCode;

    QQuickItem *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString());
        return;
    }

    QVariant result = executeJS(jsCode, item);
    qDebug()
        << Q_FUNC_INFO
        << result;
    socketReply(socket, result);
}

void QuickEnginePlatform::executeCommand_app_setAttribute(QTcpSocket *socket, const QString &elementId, const QString &attribute, const QString &value)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId << attribute << value;

    setProperty(socket, attribute, value, elementId);
}

void QuickEnginePlatform::onPropertyChanged()
{
    QObject *item = sender();
    QEventLoop *loop = item->property("WaitForPropertyChangeEventLoop").value<QEventLoop*>();
    if (!loop) {
        return;
    }
    const QString propertyName = item->property("WaitForPropertyChangePropertyName").toString();
    const QVariant propertyValue = item->property("WaitForPropertyChangePropertyValue");
    if (!propertyValue.isValid()) {
        loop->quit();
    }
    const QVariant property = item->property(propertyName.toLatin1().constData());
    if (property == propertyValue) {
        loop->quit();
    }
}
