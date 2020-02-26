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
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>

QuickEnginePlatform::QuickEnginePlatform(QObject *parent)
    : GenericEnginePlatform(parent)
{
}

QVariantList QuickEnginePlatform::findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem)
{
    QVariantList items;

    if (!parentItem) {
        parentItem = m_rootItem;
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

void QuickEnginePlatform::findByProperty(QTcpSocket *socket, const QString &propertyName, const QVariant &propertyValue, bool multiple, QQuickItem *parentItem)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << propertyName << propertyValue << multiple << parentItem;

    QVariantList items = findItemsByProperty(propertyName, propertyValue, parentItem);
    elementReply(socket, items, multiple);
}

QPointF QuickEnginePlatform::getAbsPosition(QQuickItem *item)
{
    QPointF position(item->x(), item->y());
    QPoint abs;
    if (item->parentItem()) {
        abs = m_rootItem->mapFromItem(item->parentItem(), position).toPoint();
    } else {
        abs = position.toPoint();
    }
    return abs;
}

QString QuickEnginePlatform::getText(QQuickItem *item)
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

void QuickEnginePlatform::initialize()
{
    qDebug() << Q_FUNC_INFO;


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
    m_rootWindow = qWindow;
    qDebug()
        << Q_FUNC_INFO
        << qWindow;
    m_rootItem = qWindow->contentItem();

    if (!m_rootItem) {
        qWarning()
            << Q_FUNC_INFO
            << "No root item!";
        return;
    }

    if (m_rootItem->childItems().isEmpty()) {
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

    if (!m_rootWindow) {
        qWarning()
            << Q_FUNC_INFO
            << "No window!";
        return;
    }

    m_rootWindow->show();
    m_rootWindow->raise();

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

    if (!m_rootWindow) {
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

    if (!m_rootWindow) {
        qWarning()
            << Q_FUNC_INFO
            << "No window!";
        return;
    }

    const bool isAppActive = m_rootWindow->isActive();
    socketReply(socket, isAppActive ? QStringLiteral("RUNNING_IN_FOREGROUND") : QStringLiteral("RUNNING_IN_BACKGROUND"));
}

void QuickEnginePlatform::backgroundCommand(QTcpSocket *socket, double seconds)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << seconds;

    if (!m_rootWindow) {
        qWarning()
            << Q_FUNC_INFO
            << "No window!";
        return;
    }

    const int msecs = seconds * 1000;

    m_rootWindow->lower();
    if (msecs > 0) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(msecs);
        loop.exec();
        m_rootWindow->raise();
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
    clickPoint(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2);
}

QQuickItem *QuickEnginePlatform::getItem(const QString &elementId)
{
    QQuickItem *item = nullptr;
    if (m_items.contains(elementId)) {
        item = qobject_cast<QQuickItem*>(m_items.value(elementId));
    }

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

    grabScreenshot(socket, m_rootItem, true);
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
