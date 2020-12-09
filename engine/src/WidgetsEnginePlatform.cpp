// Copyright (c) 2019-2020 Open Mobile Platform LLC.
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

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(categoryWidgetsEnginePlatform, "omp.qaengine.platform.widgets", QtInfoMsg)

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

WidgetsEnginePlatform::WidgetsEnginePlatform(QWindow *window)
    : GenericEnginePlatform(window)
{

}

void WidgetsEnginePlatform::initialize()
{
    if (!m_rootWindow) {
        return;
    }

    if (qApp->activePopupWidget()) {
        m_rootWidget = qApp->activePopupWidget();
    } else if (qApp->activeModalWidget()) {
        m_rootWidget = qApp->activeModalWidget();
    } else {
        qApp->focusWidget();
    }
    m_rootObject = m_rootWidget;

    emit ready();
}

void WidgetsEnginePlatform::getPageSourceCommand(QTcpSocket *socket)
{
    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    // TODO

    socketReply(socket, QString());
}

void WidgetsEnginePlatform::executeCommand_app_dumpInView(QTcpSocket *socket, const QString &elementId)
{
    qCDebug(categoryWidgetsEnginePlatform)
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

    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket << view << model;

    socketReply(socket, recursiveDumpModel(model, QModelIndex()));
}

void WidgetsEnginePlatform::executeCommand_app_posInView(QTcpSocket *socket, const QString &elementId, const QString &display)
{
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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

    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket << comboBox << model;

    socketReply(socket, recursiveDumpModel(model, QModelIndex()));
}

void WidgetsEnginePlatform::executeCommand_app_activateInComboBox(QTcpSocket *socket, const QString &elementId, const QString &display)
{
    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId << display;

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

    int index = comboBox->findText(display);
    if (index < 0) {
        socketReply(socket, QString(), 1);
    }

    comboBox->setCurrentIndex(index);

    socketReply(socket, QString());
}

void WidgetsEnginePlatform::executeCommand_app_activateInComboBox(QTcpSocket *socket, const QString &elementId, double idx)
{
    const int index = idx;

    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId << index;

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

    if (index >= comboBox->count()) {
        socketReply(socket, QString(), 1);
    }

    comboBox->setCurrentIndex(index);

    socketReply(socket, QString());
}

void WidgetsEnginePlatform::executeCommand_app_dumpInTabBar(QTcpSocket *socket, const QString &elementId)
{
    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QTabBar *tabBar = qobject_cast<QTabBar*>(item);
    if (!tabBar) {
        socketReply(socket, QString(), 1);
        return;
    }

    QStringList tabs;

    for (int i = 0; i < tabBar->count(); i++) {
        tabs.append(tabBar->tabText(i));
    }

    socketReply(socket, tabs);
}

void WidgetsEnginePlatform::executeCommand_app_posInTabBar(QTcpSocket *socket, const QString &elementId, const QString &display)
{
    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId << display;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QTabBar *tabBar = qobject_cast<QTabBar*>(item);
    if (!tabBar) {
        socketReply(socket, QString(), 1);
        return;
    }

    for (int i = 0; i < tabBar->count(); i++) {
        if (tabBar->tabText(i) == display) {
            QRect rect = tabBar->tabRect(i);

            const QPoint itemPos = getAbsPosition(item);
            const QPoint indexCenter(rect.center().x() + itemPos.x(), rect.center().y() + itemPos.y());

            socketReply(socket, QStringList({QString::number(indexCenter.x()), QString::number(indexCenter.y())}));
            return;
        }
    }

    socketReply(socket, QString(), 11);
}

void WidgetsEnginePlatform::executeCommand_app_posInTabBar(QTcpSocket *socket, const QString &elementId, double idx)
{
    const int index = idx;

    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId << index;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QTabBar *tabBar = qobject_cast<QTabBar*>(item);
    if (!tabBar) {
        socketReply(socket, QString(), 1);
        return;
    }

    if (index >= tabBar->count()) {
        socketReply(socket, QString(), 1);
        return;
    }

    QRect rect = tabBar->tabRect(index);

    const QPoint itemPos = getAbsPosition(item);
    const QPoint indexCenter(rect.center().x() + itemPos.x(), rect.center().y() + itemPos.y());

    socketReply(socket, QStringList({QString::number(indexCenter.x()), QString::number(indexCenter.y())}));
}

void WidgetsEnginePlatform::executeCommand_app_activateInTabBar(QTcpSocket *socket, const QString &elementId, const QString &display)
{
    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId << display;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QTabBar *tabBar = qobject_cast<QTabBar*>(item);
    if (!tabBar) {
        socketReply(socket, QString(), 1);
        return;
    }

    for (int i = 0; i < tabBar->count(); i++) {
        if (tabBar->tabText(i) == display) {
            tabBar->setCurrentIndex(i);
            socketReply(socket, QString());
            return;
        }
    }

    socketReply(socket, QString(), 1);
}

void WidgetsEnginePlatform::executeCommand_app_activateInTabBar(QTcpSocket *socket, const QString &elementId, double idx)
{
    const int index = idx;

    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId << index;

    QWidget *item = getItem(elementId);
    if (!item) {
        socketReply(socket, QString(), 1);
        return;
    }

    QTabBar *tabBar = qobject_cast<QTabBar*>(item);
    if (!tabBar) {
        socketReply(socket, QString(), 1);
        return;
    }

    if (index >= tabBar->count()) {
        socketReply(socket, QString(), 1);
        return;
    }

    tabBar->setCurrentIndex(index);
    socketReply(socket, QString());
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
    qCDebug(categoryWidgetsEnginePlatform)
        << Q_FUNC_INFO
        << item << m_rootWidget;

    QWidget *w = qobject_cast<QWidget*>(item);
    if (!w) {
        return QPoint();
    }
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
    qCDebug(categoryWidgetsEnginePlatform)
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
