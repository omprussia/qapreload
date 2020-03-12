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

    bool wasEmpty = !m_rootWindow;

    if (!window || window == m_rootWindow) {
        return;
    }

    m_rootWindow = window;
    m_rootWidget = qApp->activeWindow();
    m_rootObject = m_rootWidget;

    if (wasEmpty) {
        emit ready();
    }
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
        << item << m_rootWidget;

    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return QPoint();
    }
    qWarning()
        << Q_FUNC_INFO
        << item << w;

    if (w == m_rootWidget) {
        return QPoint();
    } else {
        QSize frameSize = w->window()->frameSize() - w->window()->size();
        qWarning()
            << Q_FUNC_INFO
            << "windowFrameSize:" << w->window()->frameSize()
            << "windowSize" << w->window()->size()
            << "frameSize" << frameSize
            << "rootPos:" << m_rootWidget->pos()
            << "itemPos:" << w->pos();
        return w->mapTo(m_rootWidget, QPoint(0, 0));
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
