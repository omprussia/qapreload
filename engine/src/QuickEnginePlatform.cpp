#include "QAEngine.hpp"
#include "QuickEnginePlatform.hpp"

#include <QDebug>
#include <QGuiApplication>
#include <QQuickItem>
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

    QString fixStrategy = strategy;
    fixStrategy = fixStrategy.remove(QChar(u' '));
    const QString methodName = QStringLiteral("findStrategy_%1").arg(fixStrategy);
    if (!QAEngine::metaInvoke(socket, this, methodName, {selector, true, QVariant::fromValue(reinterpret_cast<QObject*>(0))})) {
        findByProperty(socket, fixStrategy, selector, true, nullptr);
    }
}
