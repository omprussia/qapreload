// Copyright (c) 2020 Open Mobile Platform LLС.
#include "QAEngine.hpp"
#include "WidgetsEnginePlatform.hpp"
#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QBuffer>
#include <QComboBox>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QMenuBar>
#include <QMetaMethod>
#include <QMetaObject>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <QXmlQuery>
#include <QXmlStreamWriter>

#include <private/qwindow_p.h>
#include <private/qapplication_p.h>
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
    connect(qApp, &QApplication::focusWindowChanged, this, &WidgetsEnginePlatform::onFocusWindowChanged);
    if (qApp->activeWindow()) {
        onFocusWindowChanged(qApp->activeWindow()->windowHandle());
    }
}

void WidgetsEnginePlatform::onFocusWindowChanged(QWindow *window)
{
    bool wasEmpty = !m_rootWindow;

    if (!window) {
        return;
    }

    m_rootWindow = window;
    m_rootWidget = qApp->activeWindow();
    m_rootObject = m_rootWidget;

    m_rootWidgets.insert(window, qApp->activeWindow());

    connect(m_rootWindow, &QObject::destroyed, [this](QObject *o) {
        m_rootWidgets.remove(o);
        if (m_rootWindow != o) {
            return;
        }

        m_rootWindow = qGuiApp->topLevelWindows().last();
        m_rootWidget = m_rootWidgets.value(m_rootWindow);
        m_rootObject = m_rootWidget;
    });

    if (wasEmpty) {
        emit ready();
    }
}

void WidgetsEnginePlatform::getPageSourceCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    // TODO

    socketReply(socket, QString());
}

void WidgetsEnginePlatform::executeCommand_app_dumpInView(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(item);
    if (!view) {
        socketReply(socket, QString(), 1);
        return;
    }

    QAbstractItemModel *model = view->model();
    if (!model) {
        socketReply(socket, QString(), 1);
        return;
    }

    qDebug()
        << Q_FUNC_INFO
        << socket << view << model;

    socketReply(socket, recursiveDumpModel(model, QModelIndex()));
}

void WidgetsEnginePlatform::executeCommand_app_posInView(QTcpSocket *socket, const QString &elementId, const QString &display)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId << display;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(item);
    if (!view) {
        socketReply(socket, QString(), 1);
        return;
    }

    QAbstractItemModel *model = view->model();
    if (!model) {
        socketReply(socket, QString(), 1);
        return;
    }

    QModelIndex index = recursiveFindModel(model, QModelIndex(), display, true);
    QRect rect = view->visualRect(index);

    const QPoint itemPos = getAbsPosition(item);
    const QPoint indexCenter(rect.center().x() + itemPos.x(), rect.center().y() + itemPos.y());

    socketReply(socket, QStringList({QString::number(indexCenter.x()), QString::number(indexCenter.y())}));
}

void WidgetsEnginePlatform::executeCommand_app_clickInView(QTcpSocket *socket, const QString &elementId, const QString &display)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId << display;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(item);
    if (!view) {
        socketReply(socket, QString(), 1);
        return;
    }

    QAbstractItemModel *model = view->model();
    if (!model) {
        socketReply(socket, QString(), 1);
        return;
    }

    QModelIndex index = recursiveFindModel(model, QModelIndex(), display, true);
    QRect rect = view->visualRect(index);

    const QPoint itemPos = getAbsPosition(item);
    const QPoint indexCenter(rect.center().x() + itemPos.x(), rect.center().y() + itemPos.y());
    clickPoint(indexCenter);

    socketReply(socket, QStringList({QString::number(indexCenter.x()), QString::number(indexCenter.y())}));
}

void WidgetsEnginePlatform::executeCommand_app_scrollInView(QTcpSocket *socket, const QString &elementId, const QString &display)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId << display;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QAbstractItemView *view = qobject_cast<QAbstractItemView*>(item);
    if (!view) {
        socketReply(socket, QString(), 1);
        return;
    }

    QAbstractItemModel *model = view->model();
    if (!model) {
        socketReply(socket, QString(), 1);
        return;
    }

    QModelIndex index = recursiveFindModel(model, QModelIndex(), display, true);

    QItemSelectionModel *selectionModel = view->selectionModel();
    if (selectionModel) {
        selectionModel->select(index, QItemSelectionModel::ClearAndSelect);
    }

    view->scrollTo(index);
    view->setCurrentIndex(index);

    socketReply(socket, QString());
}

void WidgetsEnginePlatform::executeCommand_app_triggerInMenu(QTcpSocket *socket, const QString &text)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << text;

    for (QAction *a : m_rootWidget->findChildren<QAction*>()) {
        if (a->text().contains(text)) {
            QTimer::singleShot(0, a, &QAction::trigger);
            socketReply(socket, a->text());
            return;
        }
    }

    socketReply(socket, QString());
}

void WidgetsEnginePlatform::executeCommand_app_dumpInMenu(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    QStringList actions;

    for (QAction *a : m_rootWidget->findChildren<QAction*>()) {
        actions.append(a->text());
    }

    socketReply(socket, actions);
}

void WidgetsEnginePlatform::executeCommand_app_dumpInComboBox(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QComboBox *comboBox = qobject_cast<QComboBox*>(item);
    if (!comboBox) {
        socketReply(socket, QString(), 1);
        return;
    }

    QAbstractItemModel *model = comboBox->model();
    if (!model) {
        socketReply(socket, QString(), 1);
        return;
    }

    qDebug()
        << Q_FUNC_INFO
        << socket << comboBox << model;

    socketReply(socket, recursiveDumpModel(model, QModelIndex()));
}

QModelIndex WidgetsEnginePlatform::recursiveFindModel(QAbstractItemModel *model, QModelIndex index, const QString &display, bool partial)
{
    for (int r = 0; r < model->rowCount(index); r++) {
        for (int c = 0; c < model->columnCount(index); c++) {
            if (model->hasIndex(r, c, index)) {
                QModelIndex newIndex = model->index(r, c, index);
                const QString text = model->data(newIndex).toString();
                if ((partial && text.contains(display)) || text == display) {
                    return newIndex;
                }
                QModelIndex findIndex = recursiveFindModel(model, newIndex, display, partial);
                if (findIndex.isValid()) {
                    return findIndex;
                }
            }
        }
    }
    return QModelIndex();
}

QStringList WidgetsEnginePlatform::recursiveDumpModel(QAbstractItemModel *model, QModelIndex index)
{
    QStringList results;
    for (int r = 0; r < model->rowCount(index); r++) {
        for (int c = 0; c < model->columnCount(index); c++) {
            if (model->hasIndex(r, c, index)) {
                QModelIndex newIndex = model->index(r, c, index);
                results.append(model->data(newIndex).toString());
                results.append(recursiveDumpModel(model, newIndex));
            }
        }
    }
    return results;
}

QPoint WidgetsEnginePlatform::getAbsPosition(QObject *item)
{
    qDebug()
        << Q_FUNC_INFO
        << item << m_rootWidget;

    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return QPoint();
    }
    qDebug()
        << Q_FUNC_INFO
        << item << w;

    if (w == m_rootWidget) {
        return QPoint();
    } else {
        return w->mapTo(m_rootWidget, QPoint(0, 0));
    }
}

QPoint WidgetsEnginePlatform::getPosition(QObject *item)
{
    qDebug()
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
    qDebug()
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
    qDebug()
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
    qDebug()
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
    qDebug()
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
    qDebug()
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
