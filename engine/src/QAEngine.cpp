#include "QAEngine.hpp"
#include "QAService.hpp"
#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"
#include "QAPendingEvent.hpp"

#include <QDebug>

#include <QCoreApplication>
#include <QGuiApplication>

#include <QMouseEvent>
#include <QKeyEvent>

#include <QTimer>

#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickItemGrabResult>
#include <QQuickView>

#include <QDBusMessage>

#include <private/qquickwindow_p.h>
#include <private/qquickitem_p.h>

static QAEngine *s_instance = nullptr;

void QAEngine::initialize()
{
    if (s_instance) {
        return;
    }

    QGuiApplication *gui = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
    if (!gui) {
        return;
    }

    QTimer::singleShot(0, QAEngine::instance(), &QAEngine::onLateInitialization);
}

bool QAEngine::isLoaded()
{
    return s_instance;
}

void QAEngine::waitForChildrens()
{
    if (!m_rootItem) {
        return;
    }

    connect(m_rootItem, &QQuickItem::childrenChanged, this, &QAEngine::onChildrenChanged);
}

QJsonObject QAEngine::recursiveDumpTree(QQuickItem *rootItem, int depth)
{
    QJsonObject object = dumpObject(rootItem, depth);

    QJsonArray childArray;

    QQuickItemPrivate *itemPrivate = QQuickItemPrivate::get(rootItem);

    int z = 0;
    for (QQuickItem *child : itemPrivate->paintOrderChildItems()) {
        QJsonObject childObject = recursiveDumpTree(child, ++z);
        childArray.append(QJsonValue(childObject));
    }

    object.insert(QStringLiteral("children"), QJsonValue(childArray));

    return object;
}

QJsonObject QAEngine::dumpObject(QQuickItem *item, int depth)
{
    QJsonObject object;

    const QString className = QString::fromLatin1(item->metaObject()->className()).section(QChar('_'), 0, 0);

    const QString id = QStringLiteral("%1_0x%2").arg(className)
                       .arg(reinterpret_cast<quintptr>(item),
                       QT_POINTER_SIZE * 2, 16, QLatin1Char('0'));

    object.insert(QStringLiteral("id"), QJsonValue(id));
    object.insert(QStringLiteral("classname"), QJsonValue(className));

    auto mo = item->metaObject();
    do {
      std::vector<std::pair<QString, QVariant> > v;
      v.reserve(mo->propertyCount() - mo->propertyOffset());
      for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i)
          v.emplace_back(mo->property(i).name(),
                         mo->property(i).read(item));
      std::sort(v.begin(), v.end());
      for (auto &i : v) {
          if (!object.contains(i.first)
                  && i.second.canConvert<QString>()) {
              object.insert(i.first, QJsonValue::fromVariant(i.second));
          }
      }
    } while ((mo = mo->superClass()));

    QRectF rectF(item->x(), item->y(), item->width(), item->height());
    QRect rect = rectF.toRect();
    object.insert(QStringLiteral("width"), QJsonValue(rect.width()));
    object.insert(QStringLiteral("height"), QJsonValue(rect.height()));
    object.insert(QStringLiteral("x"), QJsonValue(rect.x()));
    object.insert(QStringLiteral("y"), QJsonValue(rect.y()));
    object.insert(QStringLiteral("z"), QJsonValue(depth));

    QPointF position(item->x(), item->y());
    QPoint abs;
    if (item->parentItem()) {
        abs = m_rootItem->mapFromItem(item->parentItem(), position).toPoint();
    } else {
        abs = position.toPoint();
    }

    object.insert(QStringLiteral("abs_x"), QJsonValue(abs.x()));
    object.insert(QStringLiteral("abs_y"), QJsonValue(abs.y()));

    object.insert(QStringLiteral("objectName"), QJsonValue(item->objectName()));
    object.insert(QStringLiteral("enabled"), QJsonValue(item->isEnabled()));
    object.insert(QStringLiteral("visible"), QJsonValue(item->isVisible()));

    object.insert(QStringLiteral("mainTextProperty"), getText(item));

    return object;
}

void QAEngine::sendGrabbedObject(QQuickItem *item, const QDBusMessage &message)
{
    QSharedPointer<QQuickItemGrabResult> grabber = item->grabToImage();
    connect(grabber.data(), &QQuickItemGrabResult::ready, [grabber, message]() {
        QByteArray arr;
        QBuffer buffer(&arr);
        buffer.open(QIODevice::WriteOnly);
        bool success = grabber->image().save(&buffer, "PNG");

        if (!success) {
            qWarning() << Q_FUNC_INFO << "Error saving image!";
        }

        if (arr.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "Saved empty image!";
        }

        QAService::sendMessageReply(message, arr);
    });
}

void QAEngine::onMouseEvent(QMouseEvent *event)
{
    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootItem->window());
    wp->deliverMouseEvent(event);

    if (event->type() == QEvent::MouseButtonRelease && wp->mouseGrabberItem) {
        wp->setMouseGrabber(nullptr);
    }
}

void QAEngine::onKeyEvent(QKeyEvent *event)
{
    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootItem->window());
    wp->deliverKeyEvent(event);
}

void QAEngine::onLateInitialization()
{
    setParent(qGuiApp);

    QWindowList windows = qGuiApp->topLevelWindows();
    for (QWindow *window : windows) {
        QQuickWindow *qWindow = qobject_cast<QQuickWindow*>(window);
        if (qWindow && qWindow->contentItem()) {
            m_rootItem = qWindow->contentItem();
            break;
        }
    }
    if (!m_rootItem) {
        qWarning() << Q_FUNC_INFO << "Can't find window";
        return;
    }

    m_mouseEngine = new QAMouseEngine(this);
    connect(m_mouseEngine, &QAMouseEngine::triggered, this, &QAEngine::onMouseEvent);

    m_keyEngine = new QAKeyEngine(this);
    connect(m_keyEngine, &QAKeyEngine::triggered, this, &QAEngine::onKeyEvent);

    if (m_rootItem->childItems().isEmpty()) { // probably declarative cache
        waitForChildrens(); // let's wait for loading
    } else {
        QAService::instance()->initialize();
    }
}

void QAEngine::onChildrenChanged()
{
    if (m_rootItem->childItems().isEmpty()) {
        return;
    }

    disconnect(m_rootItem, &QQuickItem::childrenChanged, this, &QAEngine::onChildrenChanged);

    QAService::instance()->initialize();
}

QString QAEngine::getText(QQuickItem *item) const
{
    static const char *textProperties[] = {
        "label",
        "title",
        "description",
        "placeholderText",
        "text",
        "value",
        "name",
    };

    for (const char *textProperty : textProperties) {
        if (item->metaObject()->indexOfProperty(textProperty) > 0) {
            return item->property(textProperty).toString();
        }
    }

    return QString();
}

QAEngine *QAEngine::instance()
{
    if (!s_instance) {
        s_instance = new QAEngine;
    }
    return s_instance;
}

QAEngine::QAEngine(QObject *parent)
    : QObject(parent)
{
}

QAEngine::~QAEngine()
{
}

void QAEngine::dumpTree(const QDBusMessage &message)
{
    QJsonObject tree = recursiveDumpTree(m_rootItem);
    QJsonDocument doc(tree);
    const QByteArray dump = doc.toJson(QJsonDocument::Compact);

    QAService::sendMessageReply(message, QString::fromUtf8(dump));
}

void QAEngine::dumpCurrentPage(const QDBusMessage &message)
{
    if (m_rootItem->childItems().isEmpty()) {
        return;
    }

    QQuickItem *pageStack = m_rootItem->childItems().first()->property("pageStack").value<QQuickItem*>();
    if (!pageStack) {
        qWarning() << Q_FUNC_INFO << "Cannot find PageStack!";
        QAService::sendMessageError(message, QStringLiteral("PageStack not found"));
        return;
    }

    QQuickItem *currentPage = pageStack->property("currentPage").value<QQuickItem*>();
    if (!currentPage) {
        qWarning() << Q_FUNC_INFO << "Cannot get currentPage from PageStack!";
        QAService::sendMessageError(message, QStringLiteral("currentPage not found"));
        return;
    }

    QJsonObject tree = recursiveDumpTree(currentPage);
    QJsonDocument doc(tree);
    const QByteArray dump = doc.toJson(QJsonDocument::Compact);

    QAService::sendMessageReply(message, QString::fromUtf8(dump));
}

void QAEngine::clickPoint(int posx, int posy, const QDBusMessage &message)
{
    connect(m_mouseEngine->click(QPointF(posx, posy)),
            &QAPendingEvent::completed, [message](){
        QAService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::pressAndHold(int posx, int posy, const QDBusMessage &message)
{
    connect(m_mouseEngine->pressAndHold(QPointF(posx, posy)),
            &QAPendingEvent::completed, [message](){
        QAService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::mouseSwipe(int startx, int starty, int stopx, int stopy, const QDBusMessage &message)
{
    connect(m_mouseEngine->move(QPointF(startx, starty), QPointF(stopx, stopy)),
            &QAPendingEvent::completed, [message](){
        QAService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::grabWindow(const QDBusMessage &message)
{
    sendGrabbedObject(m_rootItem, message);
}

void QAEngine::grabCurrentPage(const QDBusMessage &message)
{
    QQuickItem *pageStack = m_rootItem->property("pageStack").value<QQuickItem*>();
    if (!pageStack) {
        qWarning() << Q_FUNC_INFO << "Cannot find PageStack!";
        QAService::sendMessageError(message, QStringLiteral("PageStack not found"));
        return;
    }

    QQuickItem *currentPage = pageStack->property("currentPage").value<QQuickItem*>();
    if (currentPage) {
        qWarning() << Q_FUNC_INFO << "Cannot find currentPage in PageStack!";
        QAService::sendMessageError(message, QStringLiteral("currentPage not found"));
        return;
    }

    sendGrabbedObject(currentPage, message);
}

void QAEngine::pressEnter(int count, const QDBusMessage &message)
{
    connect(m_keyEngine->pressEnter(count),
            &QAPendingEvent::completed, [message](){
        QAService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::pressBackspace(int count, const QDBusMessage &message)
{
    connect(m_keyEngine->pressBackspace(count),
            &QAPendingEvent::completed, [message](){
        QAService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::pressKeys(const QString &keys, const QDBusMessage &message)
{
    connect(m_keyEngine->pressKeys(keys),
            &QAPendingEvent::completed, [message](){
        QAService::sendMessageReply(message, QVariantList());
    });
}

void QAEngine::clearFocus()
{
    if (!m_rootItem) {
        return;
    }

    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(m_rootItem->window());
    wp->clearFocusObject();
}
