#include "SailfishEnginePlatform.hpp"

#include <QDebug>
#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickWindow>

#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"

SailfishEnginePlatform::SailfishEnginePlatform(QObject *parent)
    : QuickEnginePlatform(parent)
{
    m_mouseEngine->setMode(QAMouseEngine::TouchEventMode);
}

QQuickItem *SailfishEnginePlatform::coverItem()
{
    qWarning()
        << Q_FUNC_INFO;

    QWindow *window = nullptr;
    for (QWindow *w : qGuiApp->allWindows()) {
        if (getClassName(w) == QLatin1String("DeclarativeCoverWindow")) {
            window = w;
            break;
        }
    }
    if (!window) {
        return nullptr;
    }
    QQuickWindow *qw = qobject_cast<QQuickWindow*>(window);
    return qw->contentItem();
}

QQuickItem *SailfishEnginePlatform::getPageStack()
{
    qWarning()
        << Q_FUNC_INFO;

    QQuickItem *pageStack = m_rootQuickItem->property("pageStack").value<QQuickItem*>();
    if (!pageStack) {
        pageStack = m_rootQuickItem->childItems().first()->property("pageStack").value<QQuickItem*>();
        if (!pageStack) {
            qWarning()
                << Q_FUNC_INFO
                << "Cannot find PageStack!";
            return nullptr;
        }
    }
    return pageStack;
}

QQuickItem *SailfishEnginePlatform::getCurrentPage()
{
    qWarning()
        << Q_FUNC_INFO;

    QQuickItem *pageStack = getPageStack();
    if (!pageStack) {
        return nullptr;
    }

    QQuickItem *currentPage = pageStack->property("currentPage").value<QQuickItem*>();
    if (!currentPage) {
        qWarning()
            << Q_FUNC_INFO
            << "Cannot get currentPage from PageStack!";
        return nullptr;
    }

    return currentPage;
}

void SailfishEnginePlatform::onChildrenChanged()
{
    if (m_rootQuickItem->childItems().isEmpty()) {
        return;
    }

    qDebug()
        << Q_FUNC_INFO
        << "Congratilations! New childrens appeared!";

    disconnect(m_rootQuickItem, &QQuickItem::childrenChanged,
               this, &SailfishEnginePlatform::onChildrenChanged);

    emit ready();
}

void SailfishEnginePlatform::pullDownTo(const QString &text)
{
    qDebug()
        << Q_FUNC_INFO
        << text;

    QQuickItem *page = getCurrentPage();

    QVariantList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem *flickable = flickables.first().value<QQuickItem*>();
    bool atYBeginning = flickable->property("atYBeginning").toBool();
    if (!atYBeginning) {
        QMetaObject::invokeMethod(flickable, "scrollToTop", Qt::DirectConnection);
        while (!atYBeginning) {
            QEventLoop loop;
#if QT_VERSION >= 0x051200
            connect(flickable, SIGNAL(atYBeginningChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYBeginning = flickable->property("atYBeginning").toBool();
        }
    }

    QVariantList pullDownMenus = findItemsByClassName(QStringLiteral("PullDownMenu"), page);
    pullDownMenus = filterVisibleItems(pullDownMenus);
    if (pullDownMenus.isEmpty()) {
        return;
    }
    QQuickItem *pullDownMenu = pullDownMenus.first().value<QQuickItem*>();
    QVariantList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pullDownMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = columns.first().value<QQuickItem*>();
    QVariantList items = findItemsByText(text, false, column);
    if (items.isEmpty() || items.count() > 1) {
        return;
    }
    QQuickItem *item = items.first().value<QQuickItem*>();
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() / 2;
    const int dragYEnd = dragY - itemAbs.y() + item->height();

    mouseSwipe(dragX, dragY, dragX, dragYEnd);
}

void SailfishEnginePlatform::pullDownTo(int index)
{
    qDebug()
        << Q_FUNC_INFO
        << index;

    QQuickItem *page = getCurrentPage();

    QVariantList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem *flickable = flickables.first().value<QQuickItem*>();
    bool atYBeginning = flickable->property("atYBeginning").toBool();
    if (!atYBeginning) {
        QMetaObject::invokeMethod(flickable, "scrollToTop", Qt::DirectConnection);
        while (!atYBeginning) {
            QEventLoop loop;
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
            connect(flickable, SIGNAL(atYBeginningChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYBeginning = flickable->property("atYBeginning").toBool();
        }
    }

    QVariantList pullDownMenus = findItemsByClassName(QStringLiteral("PullDownMenu"), page);
    pullDownMenus = filterVisibleItems(pullDownMenus);
    if (pullDownMenus.isEmpty()) {
        return;
    }
    QQuickItem *pullDownMenu = pullDownMenus.first().value<QQuickItem*>();
    QVariantList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pullDownMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = columns.first().value<QQuickItem*>();
    QVariantList items = findItemsByClassName(QStringLiteral("MenuItem"), column);
    if (items.isEmpty() || items.count() < (index + 1)) {
        return;
    }
    QQuickItem *item = items.at(index).value<QQuickItem*>();
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() / 2;
    const int dragYEnd = dragY - itemAbs.y() + item->height();

    mouseSwipe(dragX, dragY, dragX, dragYEnd);
}

void SailfishEnginePlatform::pushUpTo(const QString &text)
{
    qDebug()
        << Q_FUNC_INFO
        << text;

    QQuickItem *page = getCurrentPage();

    QVariantList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem *flickable = flickables.first().value<QQuickItem*>();
    bool atYEnd = flickable->property("atYEnd").toBool();
    if (!atYEnd) {
        QMetaObject::invokeMethod(flickable, "scrollToBottom", Qt::DirectConnection);
        while (!atYEnd) {
            QEventLoop loop;
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
            connect(flickable, SIGNAL(atYEndChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYEnd = flickable->property("atYEnd").toBool();
        }
    }

    QVariantList pushUpMenus = findItemsByClassName(QStringLiteral("PushUpMenu"), page);
    pushUpMenus = filterVisibleItems(pushUpMenus);
    if (pushUpMenus.isEmpty()) {
        return;
    }
    QQuickItem *pushUpMenu = pushUpMenus.first().value<QQuickItem*>();
    QVariantList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pushUpMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = columns.first().value<QQuickItem*>();
    QVariantList items = findItemsByText(text, false, column);
    if (items.isEmpty() || items.count() > 1) {
        return;
    }
    QQuickItem *item = items.first().value<QQuickItem*>();
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() / 2;
    const int dragYEnd = dragY - (itemAbs.y() - page->height() + item->height() + 100);

    mouseSwipe(dragX, dragY, dragX, dragYEnd);
}

void SailfishEnginePlatform::pushUpTo(int index)
{
    qDebug()
        << Q_FUNC_INFO
        << index;

    QQuickItem *page = getCurrentPage();

    QVariantList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem *flickable = flickables.first().value<QQuickItem*>();
    bool atYEnd = flickable->property("atYEnd").toBool();
    if (!atYEnd) {
        QMetaObject::invokeMethod(flickable, "scrollToBottom", Qt::DirectConnection);
        while (!atYEnd) {
            QEventLoop loop;
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
            connect(flickable, SIGNAL(atYEndChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYEnd = flickable->property("atYEnd").toBool();
        }
    }

    QVariantList pushUpMenus = findItemsByClassName(QStringLiteral("PushUpMenu"), page);
    pushUpMenus = filterVisibleItems(pushUpMenus);
    if (pushUpMenus.isEmpty()) {
        return;
    }
    QQuickItem *pushUpMenu = pushUpMenus.first().value<QQuickItem*>();
    QVariantList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pushUpMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = columns.first().value<QQuickItem*>();
    QVariantList items = findItemsByClassName(QStringLiteral("MenuItem"), column);
    if (items.isEmpty() || items.count() < (index + 1)) {
        return;
    }
    QQuickItem *item = items.at(index).value<QQuickItem*>();
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() / 2;
    const int dragYEnd = dragY - (itemAbs.y() - page->height() + item->height() + 100);

    mouseSwipe(dragX, dragY, dragX, dragYEnd);
}

void SailfishEnginePlatform::scrollToItem(QQuickItem *item)
{
    qDebug()
        << Q_FUNC_INFO
        << item;

    if (!item) {
        return;
    }
    QQuickItem *flickable = findParentFlickable(item);
    if (!flickable) {
        return;
    }
    QQuickItem *rootItem = getApplicationWindow();
    QPointF itemAbs = getAbsPosition(item);

    while (itemAbs.y() < 0) {
        mouseSwipe(rootItem->width() / 2, rootItem->height() * 0.05, rootItem->width() / 2, rootItem->height() * 0.95);
        itemAbs = getAbsPosition(item);
    }
    while (itemAbs.y() + item->height() > rootItem->height()) {
        mouseSwipe(rootItem->width() / 2, rootItem->height() * 0.95, rootItem->width() / 2, rootItem->height() * 0.05);
        itemAbs = getAbsPosition(item);
    }
}

QVariantList SailfishEnginePlatform::openContextMenu(QQuickItem *item)
{
    qDebug()
        << Q_FUNC_INFO
        << item;

    if (!item) {
        return QVariantList();
    }
    pressAndHoldItem(item, 1200);
    return findItemsByClassName(QStringLiteral("MenuItem"), item);
}

void SailfishEnginePlatform::clickContextMenuItem(QQuickItem *item, const QString &text, bool partial)
{
    qDebug()
        << Q_FUNC_INFO
        << item << text << partial;

    const QVariantList contextMenuItems = openContextMenu(item);
    for (const QVariant &cmItem : contextMenuItems) {
        QQuickItem *item = cmItem.value<QQuickItem*>();
        if ((partial && getText(item).contains(text)) || (!partial && getText(item) == text)) {
            clickItem(item);
            return;
        }
    }
}

void SailfishEnginePlatform::clickContextMenuItem(QQuickItem *item, int index)
{
    qDebug()
        << Q_FUNC_INFO
        << item << index;

    const QVariantList contextMenuItems = openContextMenu(item);
    if (index < 0 || index >= contextMenuItems.count()) {
        return;
    }

    clickItem(contextMenuItems.at(index).value<QQuickItem*>());
}

void SailfishEnginePlatform::initialize()
{
    qWarning()
        << Q_FUNC_INFO
        << m_rootQuickItem;

    QuickEnginePlatform::initialize();

    if (!m_rootQuickItem) {
        qWarning()
            << Q_FUNC_INFO
            << "No root item!";
        return;
    }

    qDebug()
        << Q_FUNC_INFO
        << m_rootQuickItem->window();

    if (m_rootQuickItem->childItems().isEmpty()) { // probably declarative cache
        qDebug()
            << Q_FUNC_INFO
            << "No childrens! Waiting for population...";
        connect(m_rootQuickItem, &QQuickItem::childrenChanged,
                this, &SailfishEnginePlatform::onChildrenChanged); // let's wait for loading
    }
}

void SailfishEnginePlatform::executeCommand_app_pullDownTo(QTcpSocket *socket, const QString &destination)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << destination;

    pullDownTo(destination);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_pullDownTo(QTcpSocket *socket, double destination)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << destination;

    pullDownTo(destination);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_pushUpTo(QTcpSocket *socket, const QString &destination)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << destination;

    pushUpTo(destination);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_pushUpTo(QTcpSocket *socket, double destination)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << destination;

    pushUpTo(destination);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_clickContextMenuItem(QTcpSocket *socket, const QString &elementId, const QString &destination)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId << destination;

    QQuickItem *item = getItem(elementId);
    if (item) {
        clickContextMenuItem(item, destination);
    }
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_clickContextMenuItem(QTcpSocket *socket, const QString &elementId, double destination)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId << destination;

    QQuickItem *item = getItem(elementId);
    if (item) {
        clickContextMenuItem(item, destination);
    }
    socketReply(socket, QString());
}
