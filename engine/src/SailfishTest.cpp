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
#include <QQmlEngine>

#include <private/qv4engine_p.h>
#include <private/qv8engine_p.h>

/*!
    \qmltype SailfishTest
    \inqmlmodule ru.omprussia.sailfishtest
    \brief Provides classes for Sailfish OS applications testing.

    SailfishTest is intended to provide access for testing Sailfish OS applications.
    SailfishTest allows developers to build and run streamlined and application-specific functional UI tests,
    containing a compact API to create tests and use its execution engine to automate and run those tests on real devices.

    Property values and method arguments are automatically converted between QML/Qt. There is
    limited control over this process for UI testing. For unit testing it is recommended to use C++ and
    the Qt Test module.

*/

/*!
    \qmlproperty enum SailfishTest::SwipeDirection

    \section2 This enum type describes possible values of swipe directions.

    \value SwipeDirectionUp
            Swipe up.
    \value SwipeDirectionLeft
            Swipe to the left.
    \value SwipeDirectionRight
            Swipe to the right.
    \value SwipeDirectionDown
            Swipe down.
*/

/*!
    \qmlproperty enum SailfishTest::PeekDirection

    \section2 This enum type describes possible value of peek directions.

    \value PeekDirectionUp
            Peek up.
    \value PeekDirectionLeft
            Peek to the left.
    \value PeekDirectionRight
            Peek to the right.
    \value PeekDirectionDown
            Peek down.
*/

SailfishTest::SailfishTest(QObject *parent)
    : QObject(parent)
{

}

QStringList SailfishTest::declarativeFunctions() const
{
    auto mo = metaObject();
    QSet<QString> objectFunctions;
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        objectFunctions.insert(QString::fromLatin1(mo->method(i).name()));
    }
    return objectFunctions.toList();
}

/*!
    \qmlmethod QQuickItem SailfishTest::findItemByObjectName(const QString &objectName, QQuickItem* parentItem)

    Search for single item, starting from the \a parentItem. The located items will be returned as pointer to QQuickItem.
    The function return an item whose objectName attribute matches the \a objectName.

*/

QQuickItem *SailfishTest::findItemByObjectName(const QString &objectName, QQuickItem* parentItem)
{
    return QAEngine::findItemByObjectName(objectName, parentItem);
}

/*!
    \qmlmethod QVariantList SailfishTest::findItemsByClassName(const QString &className, QQuickItem *parentItem)

    Search for multiple items, starting from the \a parentItem. The located elements will be returned as array of QQuickItem.
    The function returns array of items whose class name matches \a className.

*/

QVariantList SailfishTest::findItemsByClassName(const QString &className, QQuickItem *parentItem)
{
    return QAEngine::findItemsByClassName(className, parentItem);
}

/*!
    \qmlmethod QVariantList SailfishTest::findItemsByText(const QString &text, bool partial, QQuickItem *parentItem)

    Search for multiple items, starting from the \a parentItem. The located elements will be returned as array of QQuickItem.
    The function returns array of items whose visible text matches the \a text considering \a partial value.

*/

QVariantList SailfishTest::findItemsByText(const QString &text, bool partial, QQuickItem *parentItem)
{
    return QAEngine::findItemsByText(text, partial, parentItem);
}

/*!
    \qmlmethod QVariantList SailfishTest::findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem)

    Search for multiple items, starting from the \a parentItem. The located elements will be returned as array of QQuickItem.
    The function returns array of items whose property \a propertyName text exact matches to \a propertyValue.

*/

QVariantList SailfishTest::findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QQuickItem *parentItem)
{
    return QAEngine::findItemsByProperty(propertyName, propertyValue, parentItem);
}

/*!
    \qmlmethod QQuickItem* SailfishTest::findParentFlickable(QQuickItem *rootItem)

    The function searches through parents of \a rootItem, returns Flickable item or null.

*/

QQuickItem* SailfishTest::findParentFlickable(QQuickItem *rootItem)
{
    return QAEngine::findParentFlickable(rootItem);
}

/*!
    \qmlmethod QVariantList SailfishTest::findNestedFlickable(QQuickItem *parentItem)

    The function searches through childs of \a parentItem, returns first Flickable item or null.
*/

QVariantList SailfishTest::findNestedFlickable(QQuickItem *parentItem)
{
    return QAEngine::findNestedFlickable(parentItem);
}

/*!
    \qmlmethod void SailfishTest::clickContextMenuItem(QQuickItem *item, const QString &text, bool partial)

    The function open context menu of \a item and click element with \a text.
    If \a partial equal true context menu item text only contains searched \a text value, else it should match exact.
*/

void SailfishTest::clickContextMenuItem(QQuickItem *item, const QString &text, bool partial)
{
    const QVariantList contextMenuItems = openContextMenu(item);
    for (const QVariant &cmItem : contextMenuItems) {
        QQuickItem* item = cmItem.value<QQuickItem*>();
        if ((partial && QAEngine::getText(item).contains(text)) || (!partial && QAEngine::getText(item) == text)) {
            clickItem(item);
            return;
        }
    }
}

/*!
    \qmlmethod void SailfishTest::clickContextMenuItem(QQuickItem *item, int index)

    The function open context menu of \a item and click item with \a index.
*/

void SailfishTest::clickContextMenuItem(QQuickItem *item, int index)
{
    const QVariantList contextMenuItems = openContextMenu(item);
    if (index < 0 || index >= contextMenuItems.count()) {
        return;
    }

    clickItem(contextMenuItems.at(index).value<QQuickItem*>());
}

/*!
    \qmlmethod QVariantList SailfishTest::openContextMenu(QQuickItem *item)

    The function returns array of menu items inside context menu \a item.
*/

QVariantList SailfishTest::openContextMenu(QQuickItem *item)
{
    if (!item) {
        return QVariantList();
    }
    pressAndHold(item);
    const QVariantList contextMenus = findItemsByClassName(QStringLiteral("ContextMenu"));
    if (contextMenus.count() < 1) {
        return QVariantList();
    }
    const QVariantList columns = findItemsByClassName(QStringLiteral("QQuickColumn"), contextMenus.first().value<QQuickItem*>());
    if (columns.count() < 1) {
        return QVariantList();
    }
    return findItemsByClassName(QStringLiteral("MenuItem"), columns.first().value<QQuickItem*>());
}

/*!
    \qmlmethod QPointF SailfishTest::getAbsPosition(QQuickItem *item)

    Determine location of \a item on the application window.
    The element's coordinates are returned as QPointF value.
*/

QPointF SailfishTest::getAbsPosition(QQuickItem *item) const
{
    return QAEngine::getAbsPosition(item);
}

/*!
    \qmlmethod void SailfishTest::enterCode(const QString &code)

    This function can be used to enter \a code on special pages with keypad.
*/

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

/*!
    \qmlmethod QQuickItem SailfishTest::getCurrentPage()

    The function returns current page item.

*/

QQuickItem *SailfishTest::getCurrentPage()
{
    return QAEngine::getCurrentPage();
}

/*!
    \qmlmethod void SailfishTest::pullDownTo(const QString &text)

    PullDownMenu is the menu is positioned above the content of the view and is accessed by dragging the view down.

    The function scrolling view to the top, if not at the top already, and click to the item with \a text.
*/

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

    mouseSwipe(dragX, dragY, dragX, dragYEnd);
}

/*!
    \qmlmethod void SailfishTest::pullDownTo(int index)

    PullDownMenu is the menu is positioned above the content of the view and is accessed by dragging the view down.

    The function scrolling view to the top, if not at the top already, and click to the item with \a index.
*/

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

    mouseSwipe(dragX, dragY, dragX, dragYEnd);
}

/*!
    \qmlmethod void SailfishTest::pushUpTo(const QString &text)

    PushUpMenu is the menu is positioned at the end of the content in the view and is accessed by dragging the view past the end of the content.

    The function scrolling view to the bottom, if not at the bottom already, and click to the item with \a text.
*/

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

    mouseSwipe(dragX, dragY, dragX, dragYEnd);
}

/*!
    \qmlmethod void SailfishTest::pushUpTo(int index)

    PushUpMenu is the menu is positioned at the end of the content in the view and is accessed by dragging the view past the end of the content.

    The function scrolling view to the bottom, if not at the bottom already, and click to the item with \a index.
*/

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

    mouseSwipe(dragX, dragY, dragX, dragYEnd);
}

/*!
    \qmlmethod void SailfishTest::scrollToItem(QQuickItem *item)

    The function scrolling view to the \a item.
*/

void SailfishTest::scrollToItem(QQuickItem *item)
{
    if (!item) {
        return;
    }
    QQuickItem* flickable = findParentFlickable(item);
    if (!flickable) {
        return;
    }
    QQuickItem* rootItem = QAEngine::getApplicationWindow();
    QPointF itemAbs = getAbsPosition(item);
    if (itemAbs.y() < 0) {
        while (itemAbs.y() < 0) {
            mouseSwipe(rootItem->width() / 2, rootItem->height() * 0.05, rootItem->width() / 2, rootItem->height() * 0.95);
            itemAbs = getAbsPosition(item);
        }
    } else if (itemAbs.y() > rootItem->height()) {
        while (itemAbs.y() > rootItem->height()) {
            mouseSwipe(rootItem->width() / 2, rootItem->height() * 0.95, rootItem->width() / 2, rootItem->height() * 0.05);
            itemAbs = getAbsPosition(item);
        }
    }
}

/*!
    \qmlmethod void SailfishTest::clickItem(QQuickItem *item)

    The function simulates clicking a mouse button on an \a item.
    The position of the click is defined by center of \a item.
*/

void SailfishTest::clickItem(QQuickItem *item)
{
    if (!item) {
        return;
    }
    const QPointF itemAbs = getAbsPosition(item);
    clickPoint(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2);
}

/*!
    \qmlmethod void SailfishTest::pressAndHold(QQuickItem *item)

    The function simulates clicking and holding a mouse button on an \a item.
*/

void SailfishTest::pressAndHold(QQuickItem *item, int delay)
{
    if (!item) {
        return;
    }
    const QPointF itemAbs = getAbsPosition(item);
    pressAndHold(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2, delay);
}

/*!
    \qmlmethod void SailfishTest::clickPoint(int posx, int posy)

    The function simulates clicking a mouse button with on an window coordinates.
    The position of the click is defined by \a posx and \a posy.
    The position given by arguments is transformed into window coordinates and then delivered.

*/

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

/*!
    \qmlmethod void SailfishTest::pressAndHold(int posx, int posy)

    The function simulates clicking and holding a mouse button on an window coordinates.
    The position of the click is defined by \a posx and \a posy.
*/

void SailfishTest::pressAndHold(int posx, int posy, int delay)
{
    QEventLoop loop;
    connect(QAEngine::instance()->m_mouseEngine->pressAndHold(QPointF(posx, posy), delay),
            &QAPendingEvent::completed, &loop, &QEventLoop::quit);
    loop.exec();
}

/*!
    \qmlmethod void SailfishTest::mouseSwipe(int startx, int starty, int stopx, int stopy)

    The function press then moves the mouse pointer to the position given by coordinates on screen, and releasing press at the end.
    The position given by \a startx, \a starty, \a stopx, \a stopy is transformed into window coordinates and then delivered.
*/

void SailfishTest::mouseSwipe(int startx, int starty, int stopx, int stopy)
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

/*!
    \qmlmethod void SailfishTest::mouseDrag(int startx, int starty, int stopx, int stopy, int delay)

    The function simulates long press then dragging the mouse on an item with button pressed.
    The initial drag position is defined by \a startx and \a starty, and drop point is defined by \a stopx and \a stopy.
*/

void SailfishTest::mouseDrag(int startx, int starty, int stopx, int stopy, int delay)
{
    QEventLoop loop;
    QTimer timer;
    timer.setInterval(800);
    timer.setSingleShot(true);
    connect(QAEngine::instance()->m_mouseEngine->drag(QPointF(startx, starty),
                                                      QPointF(stopx, stopy),
                                                      delay),
            &QAPendingEvent::completed, &timer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();
}

/*!
    \qmlmethod void SailfishTest::goBack()

    The function navigates back to the previous page.
    This function simulates click to 10, 10 coordinates, you should be sure there is PageStackIndicator with back action on page.
*/

void SailfishTest::goBack()
{
    clickPoint(10, 10);
}

/*!
    \qmlmethod void SailfishTest::goForward()
    This function simulates click to window.widt - 10, 10 coordinates, you should be sure there is PageStackIndicator with forward action on page.

    The function navigates forward to the next page.
*/

void SailfishTest::goForward()
{
    QQuickItem* rootItem = QAEngine::getApplicationWindow();
    clickPoint(rootItem->width() - 10, 10);
}

/*!
    \qmlmethod void SailfishTest::swipe(SailfishTest::SwipeDirection direction)

    The function simulates generic swipe gesture with \a direction.

*/

void SailfishTest::swipe(SailfishTest::SwipeDirection direction)
{
    QQuickItem* rootItem = QAEngine::instance()->rootItem();
    QRectF rootRect(0, 0, rootItem->width(), rootItem->height());
    switch (direction) {
    case SwipeDirectionUp:
        mouseSwipe(rootRect.center().x(), rootRect.center().y(), rootRect.center().x(), 0);
        break;
    case SwipeDirectionLeft:
        mouseSwipe(rootRect.center().x(), rootRect.center().y(), 0, rootRect.center().y());
        break;
    case SwipeDirectionRight:
        mouseSwipe(rootRect.center().x(), rootRect.center().y(), rootRect.width(), rootRect.center().y());
        break;
    case SwipeDirectionDown:
        mouseSwipe(rootRect.center().x(), rootRect.center().y(), rootRect.center().x(), rootRect.height());
        break;
    default:
        break;
    }
}

/*!
    \qmlmethod void SailfishTest::peek(SailfishTest::PeekDirection direction)

    The function simulates generic peek gesture with \a direction.

*/
void SailfishTest::peek(SailfishTest::PeekDirection direction)
{
    QQuickItem* rootItem = QAEngine::instance()->rootItem();
    QRectF rootRect(0, 0, rootItem->width(), rootItem->height());
    switch (direction) {
    case PeekDirectionUp:
        mouseSwipe(rootRect.center().x(), rootRect.height(), rootRect.center().x(), 0);
        break;
    case PeekDirectionLeft:
        mouseSwipe(rootRect.width(), rootRect.center().y(), 0, rootRect.center().y());
        break;
    case PeekDirectionRight:
        mouseSwipe(0, rootRect.center().y(), rootRect.height(), rootRect.center().y());
        break;
    case PeekDirectionDown:
        mouseSwipe(rootRect.center().x(), 0, rootRect.center().x(), rootRect.height());
        break;
    default:
        break;
    }
}

/*!
    \qmlmethod void SailfishTest::pressEnter(int count)

    This function can be used to send enter key on special pages with \a count.
*/

void SailfishTest::pressEnter(int count)
{
    QAEngine::instance()->m_keyEngine->pressEnter(count);
}

/*!
    \qmlmethod void SailfishTest::pressBackspace(int count)

    This function can be used to send backspace key on special pages with \a count.
*/

void SailfishTest::pressBackspace(int count)
{
    QAEngine::instance()->m_keyEngine->pressBackspace(count);
}

/*!
    \qmlmethod void SailfishTest::pressKeys(const QString &keys)

    This function can be used to send \a keys on special pages, you can use any UTF-8 character.
*/

void SailfishTest::pressKeys(const QString &keys)
{
    QAEngine::instance()->m_keyEngine->pressKeys(keys);
}

/*!
    \qmlmethod void SailfishTest::clearFocus()

    This function can be used to clear the focus object to hide keyboard from page.

*/

void SailfishTest::clearFocus()
{
    QAEngine::instance()->clearFocus();
}

/*!
    \qmlmethod void SailfishTest::saveScreenshot(const QString &location, bool fillBackground)

    The function saves the screenshot to the file with the given \a location.

    Use function with \a fillBackground to fill screenshot with a black background.

*/

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

/*!
    \qmlmethod void SailfishTest::sleep(int msecs)

    Sleeps for \a msecs milliseconds without processing Qt events.
*/

void SailfishTest::sleep(int msecs)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(msecs);
    loop.exec();
}

/*!
    \qmlmethod void SailfishTest::waitForPageChange()

    Use the function to wait until the current page will be changed.
*/

void SailfishTest::waitForPageChange(int timeout)
{
    waitForPropertyChange(QAEngine::getPageStack(), QStringLiteral("busy"), false, timeout);
}

void SailfishTest::waitForPropertyChange(QQuickItem *item, const QString &propertyName, const QVariant &value, int timeout)
{
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
        qWarning() << Q_FUNC_INFO << item << "property" << propertyName << "have on notifySignal!";
        return;
    }
    QEventLoop loop;
    QTimer timer;
    item->setProperty("SailfishTestEventLoop", QVariant::fromValue(&loop));
    item->setProperty("SailfishTestPropertyName", propertyName);
    item->setProperty("SailfishTestPropertyValue", value);
    const QMetaMethod propertyChanged = metaObject()->method(metaObject()->indexOfSlot("onPropertyChanged()"));
    connect(item, prop.notifySignal(), this, propertyChanged);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeout);
    loop.exec();
    disconnect(item, prop.notifySignal(), this, propertyChanged);
}

void SailfishTest::onPropertyChanged()
{
    QObject *item = sender();
    QEventLoop *loop = item->property("SailfishTestEventLoop").value<QEventLoop*>();
    if (!loop) {
        return;
    }
    const QString propertyName = item->property("SailfishTestPropertyName").toString();
    const QVariant propertyValue = item->property("SailfishTestPropertyValue");
    if (!propertyValue.isValid()) {
        loop->quit();
    }
    const QVariant property = item->property(propertyName.toLatin1().constData());
    if (property == propertyValue) {
        loop->quit();
    }
}

void SailfishTest::assert(const QString &text)
{
    QQmlEngine* engine = qmlEngine(this);
    if (!engine) {
        return;
    }
    QV4::ExecutionEngine* eEngine = QV8Engine::getV4(engine);
    if (!eEngine) {
        return;
    }
    const QString functionName = eEngine->currentStackFrame().function;
    TestResult* tr = QAEngine::instance()->getTestResult(functionName);
    if (!tr) {
        return;
    }
    tr->success = false;
    tr->message = text;
    tr->raise();
}

void SailfishTest::message(const QString &text)
{
    QAEngine::print(QStringLiteral("# %1").arg(text));
}

/*!
    \qmlmethod void SailfishTest::assertEqual(const QVariant &value1, const QVariant &value2, const QString &text)

    The function checks that \a value1 and \a value2 are equal.
    If the values do not compare equal, the test will fail.

    Use \a text to generate useful default error message.
*/

void SailfishTest::assertEqual(const QVariant &value1, const QVariant &value2, const QString &text)
{
    if (!compareEqual(value1, value2)) {
        assert(text.isEmpty() ? QStringLiteral("Assert: %1 != %2").arg(value1.toString(), value2.toString()) : text);
    }
}

/*!
    \qmlmethod void SailfishTest::assertNotEqual(const QVariant &value1, const QVariant &value2, const QString &text)

    The function checks that \a value1 and \a value2 are not equal.
    If the values do compare equal, the test will fail.

    Use \a text to generate useful default error message.
*/

void SailfishTest::assertNotEqual(const QVariant &value1, const QVariant &value2, const QString &text)
{
    if (!compareNotEqual(value1, value2)) {
        assert(text.isEmpty() ? QStringLiteral("Assert: %1 == %2").arg(value1.toString(), value2.toString()) : text);
    }
}

/*!
    \qmlmethod void SailfishTest::assertTrue(bool value, const QString &text)

    The function checks that \a value is true.
    If the value do compare false, the test will fail.

    Use \a text to generate useful default error message.
*/

void SailfishTest::assertTrue(bool value, const QString &text)
{
    if (!compareTrue(value)) {
        assert(text.isEmpty() ? QStringLiteral("Assert: %1 is False").arg(value) : text);
    }
}

/*!
    \qmlmethod void SailfishTest::assertFalse(bool value, const QString &text)

    The function checks that \a value is false.
    If the value do compare true, the test will fail.

    Use \a text to generate useful default error message.
*/

void SailfishTest::assertFalse(bool value, const QString &text)
{
    if (!compareFalse(value)) {
        assert(text.isEmpty() ? QStringLiteral("Assert: %1 is True").arg(value) : text);
    }
}

/*!
    \qmlmethod void SailfishTest::compareEqual(const QVariant &value1, const QVariant &value2)

    The function checks that \a value1 and \a value2 are equal and returns bool value.
*/

bool SailfishTest::compareEqual(const QVariant &value1, const QVariant &value2)
{
    return value1 == value2;
}

/*!
    \qmlmethod void SailfishTest::compareNotEqual(const QVariant &value1, const QVariant &value2)

    The function checks that \a value1 and \a value2 are not equal and returns bool value.
*/

bool SailfishTest::compareNotEqual(const QVariant &value1, const QVariant &value2)
{
    return value1 != value2;
}

/*!
    \qmlmethod void SailfishTest::compareTrue(bool value)

    The function checks that \a value1 is True, and returns bool value.
*/

bool SailfishTest::compareTrue(bool value)
{
    return value;
}

/*!
    \qmlmethod void SailfishTest::compareFalse(bool value)

    The function checks that \a value1 is False, and returns bool value.
*/

bool SailfishTest::compareFalse(bool value)
{
    return !value;
}
