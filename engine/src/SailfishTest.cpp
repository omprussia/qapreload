#include "SailfishTest.hpp"
#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"
#include "QAEngine.hpp"
#include "QAPendingEvent.hpp"

#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QQuickItem>
#include <QQuickItemGrabResult>

QStringList SailfishTest::declarativeFunctions() const
{
    auto mo = metaObject();
    QSet<QString> objectFunctions;
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        objectFunctions.insert(QString::fromLatin1(mo->method(i).name()));
    }
    return objectFunctions.toList();
}

QQuickItem *SailfishTest::findItemByObjectName(const QString &objectName, QQuickItem* parentItem)
{
    return QAEngine::findItemByObjectName(objectName, parentItem);
}

QVariantList SailfishTest::findItemsByClassName(const QString &className, QQuickItem *parentItem)
{
    return QAEngine::findItemsByClassName(className, parentItem);
}

QVariantList SailfishTest::findItemsByText(const QString &text, bool partial, QQuickItem *parentItem)
{
    return QAEngine::findItemsByText(text, partial, parentItem);
}

QVariantList SailfishTest::findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem)
{
    return QAEngine::findItemsByProperty(propertyName, propertyValue, parentItem);
}

QQuickItem* SailfishTest::findParentFlickable(QQuickItem *rootItem)
{
    return QAEngine::findParentFlickable(rootItem);
}

QVariantList SailfishTest::findNestedFlickable(QQuickItem *parentItem)
{
    return QAEngine::findNestedFlickable(parentItem);
}

QPointF SailfishTest::getAbsPosition(QQuickItem *item) const
{
    return QAEngine::getAbsPosition(item);
}

void SailfishTest::enterCode(const QString &code)
{
    QQuickItem* keypadItem = nullptr;
    QVariantList keypads = findItemsByClassName(QStringLiteral("Keypad"));
    for (const QVariant &keypad : keypads) {
        QQuickItem *possibleKeypadItem = keypad.value<QQuickItem*>();
        if (possibleKeypadItem->isVisible()) {
            keypadItem = possibleKeypadItem;
            break;
        }
    }
    if (!keypadItem) {
        return;
    }
    QVariantList keypadButtons = findItemsByClassName(QStringLiteral("KeypadButton"), keypadItem);
    for (const QString &number : code) {
        for (const QVariant &keypadButton : keypadButtons) {
            QQuickItem *keypadItem = keypadButton.value<QQuickItem*>();
            if (keypadItem->property("text").toString() == number) {
                clickItem(keypadItem);
            }
        }
    }
}

QQuickItem *SailfishTest::getCurrentPage()
{
    return QAEngine::instance()->getCurrentPage();
}

void SailfishTest::pullDownTo(const QString &text)
{
    QQuickItem* page = getCurrentPage();

    QVariantList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem* flickable = flickables.first().value<QQuickItem*>();
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
    if (pullDownMenus.isEmpty()) {
        return;
    }
    QQuickItem* pullDownMenu = pullDownMenus.first().value<QQuickItem*>();
    QVariantList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pullDownMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = columns.first().value<QQuickItem*>();
    QVariantList items = findItemsByText(text, false, column);
    if (items.isEmpty() || items.count() > 1) {
        return;
    }
    QQuickItem* item = items.first().value<QQuickItem*>();
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = 100;
    const int dragYEnd = dragY - itemAbs.y() + item->height();

    mouseMove(dragX, dragY, dragX, dragYEnd);
}

void SailfishTest::pullDownTo(int index)
{
    QQuickItem* page = getCurrentPage();

    QVariantList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem* flickable = flickables.first().value<QQuickItem*>();
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
    if (pullDownMenus.isEmpty()) {
        return;
    }
    QQuickItem* pullDownMenu = pullDownMenus.first().value<QQuickItem*>();
    QVariantList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pullDownMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = columns.first().value<QQuickItem*>();
    QVariantList items = findItemsByClassName(QStringLiteral("MenuItem"), column);
    if (items.isEmpty() || items.count() < (index + 1)) {
        return;
    }
    QQuickItem* item = items.at(index).value<QQuickItem*>();
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = 100;
    const int dragYEnd = dragY - itemAbs.y() + item->height();

    mouseMove(dragX, dragY, dragX, dragYEnd);
}

void SailfishTest::pushUpTo(const QString &text)
{
    QQuickItem* page = getCurrentPage();

    QVariantList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem* flickable = flickables.first().value<QQuickItem*>();
    bool atYEnd = flickable->property("atYEnd").toBool();
    if (!atYEnd) {
        QMetaObject::invokeMethod(flickable, "scrollToBottom", Qt::DirectConnection);
        while (!atYEnd) {
            QEventLoop loop;
#if QT_VERSION >= 0x051200
            connect(flickable, SIGNAL(atYEndChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYEnd = flickable->property("atYEnd").toBool();
        }
    }

    QVariantList pushUpMenus = findItemsByClassName(QStringLiteral("PushUpMenu"), page);
    if (pushUpMenus.isEmpty()) {
        return;
    }
    QQuickItem* pushUpMenu = pushUpMenus.first().value<QQuickItem*>();
    QVariantList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pushUpMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = columns.first().value<QQuickItem*>();
    QVariantList items = findItemsByText(text, false, column);
    if (items.isEmpty() || items.count() > 1) {
        return;
    }
    QQuickItem* item = items.first().value<QQuickItem*>();
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() - 100;
    const int dragYEnd = dragY - (itemAbs.y() - dragY + item->height());

    mouseMove(dragX, dragY, dragX, dragYEnd);
}

void SailfishTest::pushUpTo(int index)
{
    QQuickItem* page = getCurrentPage();

    QVariantList flickables = findItemsByProperty(QStringLiteral("flickableDirection"), 2, page);
    if (flickables.isEmpty()) {
        return;
    }
    QQuickItem* flickable = flickables.first().value<QQuickItem*>();
    bool atYEnd = flickable->property("atYEnd").toBool();
    if (!atYEnd) {
        QMetaObject::invokeMethod(flickable, "scrollToBottom", Qt::DirectConnection);
        while (!atYEnd) {
            QEventLoop loop;
#if QT_VERSION >= 0x051200
            connect(flickable, SIGNAL(atYEndChanged()), &loop, SLOT(quit()));
#else
            connect(flickable, SIGNAL(isAtBoundaryChanged()), &loop, SLOT(quit()));
#endif
            loop.exec();
            atYEnd = flickable->property("atYEnd").toBool();
        }
    }

    QVariantList pushUpMenus = findItemsByClassName(QStringLiteral("PushUpMenu"), page);
    if (pushUpMenus.isEmpty()) {
        return;
    }
    QQuickItem* pushUpMenu = pushUpMenus.first().value<QQuickItem*>();
    QVariantList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), pushUpMenu);
    if (columns.isEmpty()) {
        return;
    }
    QQuickItem *column = columns.first().value<QQuickItem*>();
    QVariantList items = findItemsByClassName(QStringLiteral("MenuItem"), column);
    if (items.isEmpty() || items.count() < (index + 1)) {
        return;
    }
    QQuickItem* item = items.at(index).value<QQuickItem*>();
    const QPointF itemAbs = getAbsPosition(item);

    const int dragX = page->width() / 2;
    const int dragY = page->height() - 100;
    const int dragYEnd = dragY - (itemAbs.y() - dragY + item->height());

    mouseMove(dragX, dragY, dragX, dragYEnd);
}

void SailfishTest::scrollToItem(QQuickItem *item)
{
    if (!item) {
        return;
    }
    QQuickItem* flickable = findParentFlickable(item);
    if (!flickable) {
        return;
    }
    QQuickItem* rootItem = QAEngine::instance()->m_rootItem;
    QPointF itemAbs = getAbsPosition(item);
    if (itemAbs.y() < 0) {
        while (itemAbs.y() < 0) {
            mouseMove(rootItem->width() / 2, rootItem->height() * 0.05, rootItem->width() / 2, rootItem->height() * 0.95);
            itemAbs = getAbsPosition(item);
        }
    } else if (itemAbs.y() > rootItem->height()) {
        while (itemAbs.y() > rootItem->height()) {
            mouseMove(rootItem->width() / 2, rootItem->height() * 0.95, rootItem->width() / 2, rootItem->height() * 0.05);
            itemAbs = getAbsPosition(item);
        }
    }
}

void SailfishTest::clickItem(QQuickItem *item)
{
    if (!item) {
        return;
    }
    const QPointF itemAbs = getAbsPosition(item);
    clickPoint(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2);
}

void SailfishTest::pressAndHold(QQuickItem *item)
{
    if (!item) {
        return;
    }
    const QPointF itemAbs = getAbsPosition(item);
    pressAndHold(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2);
}

void SailfishTest::clickPoint(int posx, int posy)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(50);
    connect(QAEngine::instance()->m_mouseEngine->click(QPointF(posx, posy)),
            &QAPendingEvent::completed, &timer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();
}

void SailfishTest::pressAndHold(int posx, int posy)
{
    QEventLoop loop;
    connect(QAEngine::instance()->m_mouseEngine->pressAndHold(QPointF(posx, posy)),
            &QAPendingEvent::completed, &loop, &QEventLoop::quit);
    loop.exec();
}

void SailfishTest::mouseMove(int startx, int starty, int stopx, int stopy)
{
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(800);
    timer.setSingleShot(true);
    connect(QAEngine::instance()->m_mouseEngine->move(QPointF(startx, starty),
                                                      QPointF(stopx, stopy)),
            &QAPendingEvent::completed, &timer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();
}

void SailfishTest::goBack()
{
    clickPoint(10, 10);
}

void SailfishTest::goForward()
{
    QQuickItem* rootItem = QAEngine::instance()->m_rootItem;
    clickPoint(rootItem->width() - 10, 10);
}

void SailfishTest::swipe(SailfishTest::SwipeDirection direction)
{
    QQuickItem* rootItem = QAEngine::instance()->m_rootItem;
    QRectF rootRect(0, 0, rootItem->width(), rootItem->height());
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

void SailfishTest::peek(SailfishTest::PeekDirection direction)
{
    QQuickItem* rootItem = QAEngine::instance()->m_rootItem;
    QRectF rootRect(0, 0, rootItem->width(), rootItem->height());
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

void SailfishTest::pressEnter(int count)
{
    QAEngine::instance()->m_keyEngine->pressEnter(count);
}

void SailfishTest::pressBackspace(int count)
{
    QAEngine::instance()->m_keyEngine->pressBackspace(count);
}

void SailfishTest::pressKeys(const QString &keys)
{
    QAEngine::instance()->m_keyEngine->pressKeys(keys);
}

void SailfishTest::clearFocus()
{
    QAEngine::instance()->clearFocus();
}

void SailfishTest::saveScreenshot(const QString &location, bool fillBackground)
{
    QQuickItem* item = QAEngine::instance()->rootItem();
    QSharedPointer<QQuickItemGrabResult> grabber = item->grabToImage();
    QEventLoop loop;
    connect(grabber.data(), &QQuickItemGrabResult::ready, [grabber, fillBackground, location, &loop]() {
        if (fillBackground) {
            QPixmap pixmap(grabber->image().width(), grabber->image().height());
            QPainter painter(&pixmap);
            painter.fillRect(0, 0, pixmap.width(), pixmap.height(), Qt::black);
            painter.drawImage(0, 0, grabber->image());
            pixmap.save(location, "PNG");
        } else {
            grabber->image().save(location, "PNG");
        }
        loop.quit();
    });
    loop.exec();
}

void SailfishTest::sleep(int msecs)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(msecs);
    loop.exec();
}
