// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "SailfishEnginePlatform.hpp"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QQuickItem>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>

#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"
#include "QAEngine.hpp"

#include <mlite5/MGConfItem>

#include <QtDBus/QDBusConnection>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(categorySailfishEnginePlatform, "omp.qaengine.platform.sailfish", QtWarningMsg)

SailfishEnginePlatform::SailfishEnginePlatform(QWindow *window)
    : QuickEnginePlatform(window)
{
    m_mouseEngine->setMode(QAMouseEngine::TouchEventMode);
}

QQuickItem *SailfishEnginePlatform::getCoverItem()
{
    qCDebug(categorySailfishEnginePlatform)
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
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO;

    QQuickItem *pageStack = m_rootQuickItem->property("pageStack").value<QQuickItem*>();
    if (!pageStack) {
        pageStack = m_rootQuickItem->childItems().first()->property("pageStack").value<QQuickItem*>();
        if (!pageStack) {
            qCWarning(categorySailfishEnginePlatform)
                << Q_FUNC_INFO
                << "Cannot find PageStack!";
            return nullptr;
        }
    }
    return pageStack;
}

QQuickItem *SailfishEnginePlatform::getCurrentPage()
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO;

    QQuickItem *pageStack = getPageStack();
    if (!pageStack) {
        return nullptr;
    }

    QQuickItem *currentPage = pageStack->property("currentPage").value<QQuickItem*>();
    if (!currentPage) {
        qCWarning(categorySailfishEnginePlatform)
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

    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << "Congratilations! New childrens appeared!";

    disconnect(m_rootQuickItem, &QQuickItem::childrenChanged,
               this, &SailfishEnginePlatform::onChildrenChanged);

    emit ready();
}

void SailfishEnginePlatform::getScreenshotCoverCommand(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    grabScreenshot(socket, getCoverItem(), true);
}

void SailfishEnginePlatform::bridgeStatusChanged(const QString &interface, const QVariantMap &properties, const QStringList &)
{

    if (interface != QLatin1String("org.freedesktop.systemd1.Unit")) {
        return;
    }

    if (properties.contains(QStringLiteral("ActiveState"))) {
        const QString activeState = properties.value(QStringLiteral("ActiveState")).toString();
        qCDebug(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << activeState;

        if (activeState == QLatin1String("active")) {
            QTimer::singleShot(1000, this, &SailfishEnginePlatform::ready);
        }
    }
}

void SailfishEnginePlatform::pullDownTo(const QString &text)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << text;

    QQuickItem *page = getCurrentPage();

    QObjectList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem *flickable = qobject_cast<QQuickItem*>(flickables.first());
    bool atYBeginning = flickable->property("atYBeginning").toBool();
    if (!atYBeginning) {
        QMetaObject::invokeMethod(flickable, "scrollToTop", Qt::DirectConnection);
        while (!atYBeginning) {
            QEventLoop loop;
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
            connect(flickable, SIGNAL(atYBeginningChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYBeginning = flickable->property("atYBeginning").toBool();
        }
    }

    QObjectList pullDownMenus = findItemsByClassName(QStringLiteral("PullDownMenu"), page);
    pullDownMenus = filterVisibleItems(pullDownMenus);
    if (pullDownMenus.isEmpty()) {
        return;
    }
    QQuickItem *pullDownMenu = qobject_cast<QQuickItem*>(pullDownMenus.first());
    QObjectList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pullDownMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = qobject_cast<QQuickItem*>(columns.first());
    QObjectList items = findItemsByText(text, false, column);
    if (items.isEmpty() || items.count() > 1) {
        return;
    }
    QQuickItem *item = qobject_cast<QQuickItem*>(items.first());
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() / 2;
    const int dragYEnd = dragY - itemAbs.y() + item->height();

    mouseMove(dragX, dragY, dragX, dragYEnd);
}

void SailfishEnginePlatform::pullDownTo(int index)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << index;

    QQuickItem *page = getCurrentPage();

    QObjectList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem *flickable = qobject_cast<QQuickItem*>(flickables.first());
    bool atYBeginning = flickable->property("atYBeginning").toBool();
    if (!atYBeginning) {
        QMetaObject::invokeMethod(flickable, "scrollToTop", Qt::DirectConnection);
        while (!atYBeginning) {
            QEventLoop loop;
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
            connect(flickable, SIGNAL(atYBeginningChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYBeginning = flickable->property("atYBeginning").toBool();
        }
    }

    QObjectList pullDownMenus = findItemsByClassName(QStringLiteral("PullDownMenu"), page);
    pullDownMenus = filterVisibleItems(pullDownMenus);
    if (pullDownMenus.isEmpty()) {
        return;
    }
    QQuickItem *pullDownMenu = qobject_cast<QQuickItem*>(pullDownMenus.first());
    QObjectList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pullDownMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = qobject_cast<QQuickItem*>(columns.first());
    QObjectList items = findItemsByClassName(QStringLiteral("MenuItem"), column);
    if (items.isEmpty() || items.count() < (index + 1)) {
        return;
    }
    QQuickItem *item = qobject_cast<QQuickItem*>(items.at(index));
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() / 2;
    const int dragYEnd = dragY - itemAbs.y() + item->height();

    mouseMove(dragX, dragY, dragX, dragYEnd);
}

void SailfishEnginePlatform::pushUpTo(const QString &text)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << text;

    QQuickItem *page = getCurrentPage();

    QObjectList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem *flickable = qobject_cast<QQuickItem*>(flickables.first());
    bool atYEnd = flickable->property("atYEnd").toBool();
    if (!atYEnd) {
        QMetaObject::invokeMethod(flickable, "scrollToBottom", Qt::DirectConnection);
        while (!atYEnd) {
            QEventLoop loop;
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
            connect(flickable, SIGNAL(atYEndChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYEnd = flickable->property("atYEnd").toBool();
        }
    }

    QObjectList pushUpMenus = findItemsByClassName(QStringLiteral("PushUpMenu"), page);
    pushUpMenus = filterVisibleItems(pushUpMenus);
    if (pushUpMenus.isEmpty()) {
        return;
    }
    QQuickItem *pushUpMenu = qobject_cast<QQuickItem*>(pushUpMenus.first());
    QObjectList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pushUpMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = qobject_cast<QQuickItem*>(columns.first());
    QObjectList items = findItemsByText(text, false, column);
    if (items.isEmpty() || items.count() > 1) {
        return;
    }
    QQuickItem *item = qobject_cast<QQuickItem*>(items.first());
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() / 2;
    const int dragYEnd = dragY - (itemAbs.y() - page->height() + item->height() + 100);

    mouseMove(dragX, dragY, dragX, dragYEnd);
}

void SailfishEnginePlatform::pushUpTo(int index)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << index;

    QQuickItem *page = getCurrentPage();

    QObjectList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem *flickable = qobject_cast<QQuickItem*>(flickables.first());
    bool atYEnd = flickable->property("atYEnd").toBool();
    if (!atYEnd) {
        QMetaObject::invokeMethod(flickable, "scrollToBottom", Qt::DirectConnection);
        while (!atYEnd) {
            QEventLoop loop;
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
            connect(flickable, SIGNAL(atYEndChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYEnd = flickable->property("atYEnd").toBool();
        }
    }

    QObjectList pushUpMenus = findItemsByClassName(QStringLiteral("PushUpMenu"), page);
    pushUpMenus = filterVisibleItems(pushUpMenus);
    if (pushUpMenus.isEmpty()) {
        return;
    }
    QQuickItem *pushUpMenu = qobject_cast<QQuickItem*>(pushUpMenus.first());
    QObjectList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pushUpMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = qobject_cast<QQuickItem*>(columns.first());
    QObjectList items = findItemsByClassName(QStringLiteral("MenuItem"), column);
    if (items.isEmpty() || items.count() < (index + 1)) {
        return;
    }
    QQuickItem *item = qobject_cast<QQuickItem*>(items.at(index));
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() / 2;
    const int dragYEnd = dragY - (itemAbs.y() - page->height() + item->height() + 100);

    mouseMove(dragX, dragY, dragX, dragYEnd);
}

void SailfishEnginePlatform::scrollToItem(QQuickItem *item)
{
    qCDebug(categorySailfishEnginePlatform)
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
        mouseMove(rootItem->width() / 2, rootItem->height() * 0.05, rootItem->width() / 2, rootItem->height() * 0.95);
        itemAbs = getAbsPosition(item);
    }
    while (itemAbs.y() + item->height() > rootItem->height()) {
        mouseMove(rootItem->width() / 2, rootItem->height() * 0.95, rootItem->width() / 2, rootItem->height() * 0.05);
        itemAbs = getAbsPosition(item);
    }
}

QObjectList SailfishEnginePlatform::openContextMenu(QQuickItem *item)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << item;

    if (!item) {
        return QObjectList();
    }
    pressAndHoldItem(item, 1200);
    return findItemsByClassName(QStringLiteral("MenuItem"), item);
}

void SailfishEnginePlatform::clickContextMenuItem(QQuickItem *item, const QString &text, bool partial)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << item << text << partial;

    QObjectList contextMenuItems = openContextMenu(item);
    for (QObject *o : contextMenuItems) {
        QQuickItem *item = qobject_cast<QQuickItem*>(o);
        if ((partial && getText(item).contains(text)) || (!partial && getText(item) == text)) {
            clickItem(item);
            return;
        }
    }
}

void SailfishEnginePlatform::clickContextMenuItem(QQuickItem *item, int index)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << item << index;

    QObjectList contextMenuItems = openContextMenu(item);
    if (index < 0 || index >= contextMenuItems.count()) {
        return;
    }

    clickItem(qobject_cast<QQuickItem*>(contextMenuItems.at(index)));
}

void SailfishEnginePlatform::waitForPageChange(int timeout)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << timeout;

    waitForPropertyChange(getPageStack(), QStringLiteral("currentPage"), QVariant(), timeout);
    waitForPropertyChange(getPageStack(), QStringLiteral("busy"), false, timeout);
}

void SailfishEnginePlatform::swipe(SailfishEnginePlatform::SwipeDirection direction)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << direction;

    QRectF rootRect(0, 0, m_rootQuickItem->width(), m_rootQuickItem->height());
    switch (direction) {
    case SwipeDirectionUp:
        mouseMove(rootRect.center().x(), rootRect.center().y(), rootRect.center().x(), 0);
        break;
    case SwipeDirectionLeft:
        mouseMove(rootRect.center().x(), rootRect.center().y(), 0, rootRect.center().y());
        break;
    case SwipeDirectionRight:
        mouseMove(rootRect.center().x(), rootRect.center().y(), rootRect.width(), rootRect.center().y());
        break;
    case SwipeDirectionDown:
        mouseMove(rootRect.center().x(), rootRect.center().y(), rootRect.center().x(), rootRect.height());
        break;
    default:
        break;
    }
}

void SailfishEnginePlatform::peek(SailfishEnginePlatform::PeekDirection direction)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << direction;

    QRectF rootRect(0, 0, m_rootQuickItem->width(), m_rootQuickItem->height());
    switch (direction) {
    case PeekDirectionUp:
        mouseMove(rootRect.center().x(), rootRect.height(), rootRect.center().x(), 0);
        break;
    case PeekDirectionLeft:
        mouseMove(rootRect.width(), rootRect.center().y(), 0, rootRect.center().y());
        break;
    case PeekDirectionRight:
        mouseMove(0, rootRect.center().y(), rootRect.height(), rootRect.center().y());
        break;
    case PeekDirectionDown:
        mouseMove(rootRect.center().x(), 0, rootRect.center().x(), rootRect.height());
        break;
    default:
        break;
    }
}

void SailfishEnginePlatform::enterCode(const QString &code)
{
    QQuickItem *keypadItem = nullptr;
    QObjectList keypads = findItemsByClassName(QStringLiteral("Keypad"));
    for (QObject *ko : keypads) {
        QQuickItem *possibleKeypadItem = qobject_cast<QQuickItem*>(ko);
        if (possibleKeypadItem->isVisible()) {
            keypadItem = possibleKeypadItem;
            break;
        }
    }
    if (!keypadItem) {
        return;
    }
    QObjectList keypadButtons = findItemsByClassName(QStringLiteral("KeypadButton"), keypadItem);
    for (const QString &number : code) {
        for (QObject *kb : keypadButtons) {
            QQuickItem *keypadItem = qobject_cast<QQuickItem*>(kb);
            if (keypadItem->property("text").toString() == number) {
                clickItem(keypadItem);
            }
        }
    }
}

void SailfishEnginePlatform::goBack()
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO;

    clickPoint(10, 10);
}

void SailfishEnginePlatform::goForward()
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO;

    QQuickItem *rootItem = getApplicationWindow();
    clickPoint(rootItem->width() - 10, 10);
}

void SailfishEnginePlatform::initialize()
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO;

    QDBusConnection::systemBus().connect(
        QString(),
        QStringLiteral("/org/freedesktop/systemd1/unit/qabridge_2eservice"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("PropertiesChanged"),
        this,
        SLOT(bridgeStatusChanged(QString, QVariantMap, QStringList)));

    QFile blacklist(QStringLiteral("/etc/qapreload-blacklist"));
    if (blacklist.exists() && blacklist.open(QFile::ReadOnly)) {
        const QStringList list = QString::fromLatin1(blacklist.readAll().trimmed()).split(QChar(u'\n'));
        for (const QString &item : list) {
            const QStringList parts = item.split(QStringLiteral("::"));
            QStringList blacklisted = m_blacklistedProperties.value(parts.first());
            blacklisted.append(parts.last());
            m_blacklistedProperties.insert(parts.first(), blacklisted);
        }
        qCDebug(categorySailfishEnginePlatform)
            << "Blacklist:"
            << m_blacklistedProperties;
    }

    if (!m_rootWindow) {
        qCDebug(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << "No windows!";
        return;
    }

    QQuickWindow *qWindow = qobject_cast<QQuickWindow*>(m_rootWindow);

    if (!qWindow) {
        qCWarning(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << m_rootWindow << "is not QQuickWindow!";
        return;
    }
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << qWindow;

    m_rootQuickWindow = qWindow;
    m_rootQuickItem = qWindow->contentItem();
    m_rootObject = m_rootQuickItem;

    if (!m_rootQuickItem) {
        qCWarning(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << "No root item!";
        return;
    }

    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << m_rootQuickItem->window();

    if (m_rootQuickItem->childItems().isEmpty()) { // probably declarative cache
        qCDebug(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << "No childrens! Waiting for population...";
        connect(m_rootQuickItem, &QQuickItem::childrenChanged,
                this, &SailfishEnginePlatform::onChildrenChanged); // let's wait for loading
    } else {
        emit ready();
    }
}

void SailfishEnginePlatform::activateAppCommand(QTcpSocket *socket, const QString &appName)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << appName;

    if (appName != QAEngine::processName()) {
        qCWarning(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << appName << "is not" << QAEngine::processName();
        socketReply(socket, QString(), 1);
        return;
    }

    if (!m_rootWindow) {
        qCWarning(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << "No window!";
        return;
    }

    if (!m_rootQuickItem) {
        qCWarning(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << "No root item!";
        return;
    }

    if (m_rootQuickItem->childItems().isEmpty()) {
        qCWarning(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << "No child items!";
        return;
    }

    m_rootWindow->raise();
    m_rootWindow->requestActivate();
    QMetaObject::invokeMethod(m_rootQuickItem->childItems().first(), "activate", Qt::QueuedConnection);

    socketReply(socket, QString());
}

void SailfishEnginePlatform::queryAppStateCommand(QTcpSocket *socket, const QString &appName)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << appName;

    if (appName != QAEngine::processName()) {
        qCWarning(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << appName << "is not" << QAEngine::processName();
        socketReply(socket, QString(), 1);
        return;
    }

    if (!m_rootWindow) {
        qCWarning(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << "No window!";
        return;
    }

    qCDebug(categorySailfishEnginePlatform) << m_rootQuickItem;
    if (m_rootQuickItem->childItems().isEmpty()) {
        qCWarning(categorySailfishEnginePlatform)
            << Q_FUNC_INFO
            << "No children!";

        socketReply(socket, QStringLiteral("NOT_RUNNING"));
    } else {
        qCDebug(categorySailfishEnginePlatform) << m_rootQuickWindow;
    }

    const bool isAppActive = m_rootWindow->isActive();
    socketReply(socket, isAppActive ? QStringLiteral("RUNNING_IN_FOREGROUND") : QStringLiteral("RUNNING_IN_BACKGROUND"));
}

void SailfishEnginePlatform::backCommand(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    goBack();
    socketReply(socket, QString());
}

void SailfishEnginePlatform::forwardCommand(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    goForward();
    socketReply(socket, QString());
}

void SailfishEnginePlatform::getOrientationCommand(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    QQuickItem *item = getApplicationWindow();
    const int deviceOrientation = item->property("deviceOrientation").toInt();
    socketReply(socket, deviceOrientation == 2 || deviceOrientation == 8 ? QStringLiteral("LANDSCAPE") : QStringLiteral("PORTRAIT"));
}

void SailfishEnginePlatform::setOrientationCommand(QTcpSocket *socket, const QString &orientation)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << orientation;

    QQuickItem *item = getApplicationWindow();
    item->setProperty("deviceOrientation", orientation == QLatin1String("LANDSCAPE") ? 2 : 1);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::hideKeyboardCommand(QTcpSocket *socket, const QString &strategy, const QString &key, double keyCode, const QString &keyName)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << strategy << key << keyCode << keyName;

    clearFocus();
    socketReply(socket, QString());
}

void SailfishEnginePlatform::isKeyboardShownCommand(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    QInputMethod *ime = qApp->inputMethod();
    if (!ime) {
        socketReply(socket, false, 1);
    }
    socketReply(socket, ime->isVisible());
}

void SailfishEnginePlatform::activateIMEEngineCommand(QTcpSocket *socket, const QVariant &engine)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << engine;

    const QString layout = engine.toString();
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << layout;
    MGConfItem enabled(QStringLiteral("/sailfish/text_input/enabled_layouts"));
    if (!enabled.value().toStringList().contains(layout)) {
        socketReply(socket, QString());
        return;
    }

    MGConfItem active(QStringLiteral("/sailfish/text_input/active_layout"));
    active.set(layout);
}

void SailfishEnginePlatform::availableIMEEnginesCommand(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    MGConfItem enabled(QStringLiteral("/sailfish/text_input/enabled_layouts"));
    socketReply(socket, enabled.value().toStringList());
}

void SailfishEnginePlatform::getActiveIMEEngineCommand(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;
}

void SailfishEnginePlatform::deactivateIMEEngineCommand(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    MGConfItem active(QStringLiteral("/sailfish/text_input/active_layout"));
    socketReply(socket, active.value().toString());
}

void SailfishEnginePlatform::isIMEActivatedCommand(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    socketReply(socket, qApp->inputMethod() ? true : false);
}

void SailfishEnginePlatform::executeCommand_app_pullDownTo(QTcpSocket *socket, const QString &destination)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << destination;

    pullDownTo(destination);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_pullDownTo(QTcpSocket *socket, double destination)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << destination;

    pullDownTo(destination);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_pushUpTo(QTcpSocket *socket, const QString &destination)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << destination;

    pushUpTo(destination);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_pushUpTo(QTcpSocket *socket, double destination)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << destination;

    pushUpTo(destination);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_clickContextMenuItem(QTcpSocket *socket, const QString &elementId, const QString &destination)
{
    qCDebug(categorySailfishEnginePlatform)
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
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId << destination;

    QQuickItem *item = getItem(elementId);
    if (item) {
        clickContextMenuItem(item, destination);
    }
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_waitForPageChange(QTcpSocket *socket, double timeout)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << timeout;

    waitForPageChange(timeout);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_swipe(QTcpSocket *socket, const QString &directionString)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << directionString;

    SwipeDirection direction = SwipeDirectionDown;
    if (directionString == QLatin1String("down")) {
        direction = SwipeDirectionDown;
    } else if (directionString == QLatin1String("up")) {
        direction = SwipeDirectionUp;
    } else if (directionString == QLatin1String("left")) {
        direction = SwipeDirectionLeft;
    } else if (directionString == QLatin1String("right")) {
        direction = SwipeDirectionRight;
    }
    swipe(direction);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_peek(QTcpSocket *socket, const QString &directionString)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << directionString;

    PeekDirection direction = PeekDirectionDown;
    if (directionString == QLatin1String("down")) {
        direction = PeekDirectionDown;
    } else if (directionString == QLatin1String("up")) {
        direction = PeekDirectionUp;
    } else if (directionString == QLatin1String("left")) {
        direction = PeekDirectionLeft;
    } else if (directionString == QLatin1String("right")) {
        direction = PeekDirectionRight;
    }
    peek(direction);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_goBack(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    goBack();
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_goForward(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    goForward();
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_enterCode(QTcpSocket *socket, const QString &code)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << code;

    enterCode(code);
    socketReply(socket, QString());
}

void SailfishEnginePlatform::executeCommand_app_scrollToItem(QTcpSocket *socket, const QString &elementId)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << elementId;

    QQuickItem *item = getItem(elementId);
    if (item) {
        scrollToItem(item);
    }
    socketReply(socket, QString());

}

void SailfishEnginePlatform::executeCommand_app_saveScreenshot(QTcpSocket *socket, const QString &fileName)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket << fileName;

    const QString initialPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    const QString screenShotPath = QStringLiteral("%1/Screenshots/%2").arg(initialPath, fileName);
    QFileInfo screenshot(screenShotPath);
    QDir screnshotDir = screenshot.dir();
    if (!screnshotDir.exists()) {
        screnshotDir.mkpath(QStringLiteral("."));
    }

    QDBusMessage screenShot = QDBusMessage::createMethodCall(
                QStringLiteral("org.nemomobile.lipstick"),
                QStringLiteral("/org/nemomobile/lipstick/screenshot"),
                QStringLiteral("org.nemomobile.lipstick"),
                QStringLiteral("saveScreenshot"));
    screenShot.setArguments({ screenShotPath });
    QDBusReply<void> reply = QDBusConnection::sessionBus().call(screenShot);

    if (reply.error().type() == QDBusError::NoError) {
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void SailfishEnginePlatform::executeCommand_app_dumpCurrentPage(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    QQuickItem *currentPage = getCurrentPage();
    if (!currentPage) {
        socketReply(socket, QString());
        return;
    }
    QJsonObject reply = recursiveDumpTree(currentPage);
    socketReply(socket, QJsonDocument(reply).toJson(QJsonDocument::Compact));
}

void SailfishEnginePlatform::executeCommand_app_dumpCover(QTcpSocket *socket)
{
    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << socket;

    QQuickItem *coverItem = getCoverItem();
    if (!coverItem) {
        socketReply(socket, QString());
    } else {
        QJsonObject reply = recursiveDumpTree(coverItem);
        socketReply(socket, QJsonDocument(reply).toJson(QJsonDocument::Compact));
    }
}

void SailfishEnginePlatform::findStrategy_classname(QTcpSocket *socket, const QString &selector, bool multiple, QObject *parentItem)
{
    QObjectList items;
    if (selector == QLatin1String("DeclarativeCover")) {
        items.append(getCoverItem());
    } else if (selector == QLatin1String("QQuickRootItem")) {
        for (QWindow *window : qGuiApp->allWindows()) {
            QQuickWindow *qw = qobject_cast<QQuickWindow*>(window);
            if (!qw) {
                continue;
            }
            items.append(qw->contentItem());
        }
    } else {
        items = findItemsByClassName(selector, parentItem);
    }

    qCDebug(categorySailfishEnginePlatform)
        << Q_FUNC_INFO
        << selector << multiple << items;
    elementReply(socket, items, multiple);
}
