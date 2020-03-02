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

}

void SailfishEnginePlatform::pushUpTo(const QString &text)
{
    qDebug()
        << Q_FUNC_INFO
        << text;

}

void SailfishEnginePlatform::pushUpTo(int index)
{

}

void SailfishEnginePlatform::scrollToItem(QQuickItem *item)
{

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
}

void SailfishEnginePlatform::executeCommand_app_pullDownTo(QTcpSocket *socket, double destination)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << destination;
}

void SailfishEnginePlatform::executeCommand_app_pushUpTo(QTcpSocket *socket, const QString &destination)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << destination;
}

void SailfishEnginePlatform::executeCommand_app_pushUpTo(QTcpSocket *socket, double destination)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << destination;
}
