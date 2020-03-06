#include "QAEngine.hpp"
#include "WidgetsEnginePlatform.hpp"
#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"

#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaMethod>
#include <QMetaObject>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <QXmlQuery>
#include <QXmlStreamWriter>

#include <private/qwindow_p.h>
#include <private/qwidget_p.h>

QList<QObject *> WidgetsEnginePlatform::childrenList(QObject *parentItem)
{
    QList<QObject*> result;
    for (QWidget *w : parentItem->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        result.append(w);
    }
    return result;
}

QObject *WidgetsEnginePlatform::getParent(QObject *item)
{
    if (!item) {
        return nullptr;
    }
    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return item->parent();
    }
    return w->parentWidget();
}

QWidget *WidgetsEnginePlatform::getItem(const QString &elementId)
{
    return qobject_cast<QWidget*>(getObject(elementId));
}

WidgetsEnginePlatform::WidgetsEnginePlatform(QObject *parent)
    : GenericEnginePlatform(parent)
{

}

void WidgetsEnginePlatform::initialize()
{
    m_rootWindow = QApplication::focusWindow();
    connect(qApp, &QApplication::focusWindowChanged, this, &WidgetsEnginePlatform::onFocusWindowChanged);
    onFocusWindowChanged(m_rootWindow);
}

void WidgetsEnginePlatform::onFocusWindowChanged(QWindow *window)
{
    qDebug()
        << Q_FUNC_INFO
        << window << qApp->activeWindow();

    if (!window || window == m_rootWindow) {
        return;
    }

    m_rootWindow = window;
    m_rootWidget = qApp->activeWindow();
    m_rootObject = m_rootWidget;

    emit ready();
}

void WidgetsEnginePlatform::getLocationCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem(elementId);
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

void WidgetsEnginePlatform::getLocationInViewCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem(elementId);
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

void WidgetsEnginePlatform::getAttributeCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << attribute << elementId;

    QWidget *item = getItem(elementId);
    if (item) {
        const QVariant reply = item->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void WidgetsEnginePlatform::getPropertyCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << attribute << elementId;

    QWidget *item = getItem(elementId);
    if (item) {
        const QVariant reply = item->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void WidgetsEnginePlatform::getTextCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem(elementId);
    if (item) {
        socketReply(socket, getText(item));
    } else {
        socketReply(socket, QString());
    }
}

void WidgetsEnginePlatform::getElementScreenshotCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem(elementId);
    if (item) {
        grabScreenshot(socket, item, true);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void WidgetsEnginePlatform::getScreenshotCommand(QTcpSocket *socket)
{
    qWarning()
        << Q_FUNC_INFO
        << socket;

    grabScreenshot(socket, m_rootWidget, true);
}

void WidgetsEnginePlatform::elementEnabledCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("enabled"), elementId);
}

void WidgetsEnginePlatform::elementDisplayedCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("visible"), elementId);
}

void WidgetsEnginePlatform::elementSelectedCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("checked"), elementId);
}

void WidgetsEnginePlatform::getSizeCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem(elementId);
    if (item) {
        QJsonObject reply;
        reply.insert(QStringLiteral("width"), item->width());
        reply.insert(QStringLiteral("height"), item->height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void WidgetsEnginePlatform::setValueImmediateCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
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

void WidgetsEnginePlatform::replaceValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    setValueImmediateCommand(socket, value, elementId);
}

void WidgetsEnginePlatform::setValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    setValueImmediateCommand(socket, value, elementId);
}

void WidgetsEnginePlatform::clickCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem(elementId);
    if (item) {
        clickItem(item);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void WidgetsEnginePlatform::clearCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    setProperty(socket, QStringLiteral("text"), QString(), elementId);
}

void WidgetsEnginePlatform::submitCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    m_keyEngine->pressEnter(1);
    socketReply(socket, QString());
}

void WidgetsEnginePlatform::getPageSourceCommand(QTcpSocket *socket)
{
    qWarning()
        << Q_FUNC_INFO
        << socket;

    // TODO

    socketReply(socket, QString());
}

QPoint WidgetsEnginePlatform::getAbsPosition(QObject *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return QPoint();
    }
    if (w == m_rootWidget) {
        return QPoint();
    } else {
        QSize frameSize = w->window()->frameSize() - w->window()->size();
        return w->mapToGlobal(-m_rootWidget->pos()) - QPoint(frameSize.width(), frameSize.height());
    }
}

QPoint WidgetsEnginePlatform::getPosition(QObject *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return QPoint();
    }
    return w->pos();
}

QSize WidgetsEnginePlatform::getSize(QObject *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return QSize();
    }
    return w->size();
}

bool WidgetsEnginePlatform::isItemEnabled(QObject *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return false;
    }
    return w->isEnabled();
}

bool WidgetsEnginePlatform::isItemVisible(QObject *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return false;
    }
    return w->isVisible();
}

void WidgetsEnginePlatform::grabScreenshot(QTcpSocket *socket, QObject *item, bool fillBackground)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << item << fillBackground;

    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return;
    }

    QByteArray arr;
    QBuffer buffer(&arr);
    QPixmap pix = w->grab();
    if (fillBackground) {
        QPixmap pixmap(pix.width(), pix.height());
        QPainter painter(&pixmap);
        painter.fillRect(0, 0, pixmap.width(), pixmap.height(), Qt::black);
        painter.drawPixmap(0, 0, pix);
        pixmap.save(&buffer, "PNG");
    } else {
        pix.save(&buffer, "PNG");
    }

    socketReply(socket, arr.toBase64());
}

void WidgetsEnginePlatform::pressAndHoldItem(QObject *qitem, int delay)
{
    qWarning()
        << Q_FUNC_INFO
        << qitem << delay;

    if (!qitem) {
        return;
    }

    QWidget *item = qobject_cast<QWidget*>(qitem);
    if (!item) {
        return;
    }

    const QPointF itemAbs = getAbsPosition(item);
    pressAndHold(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2, delay);
}
