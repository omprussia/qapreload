// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "GenericEnginePlatform.hpp"
#include "QAEngine.hpp"
#include "QAKeyEngine.hpp"
#include "QAMouseEngine.hpp"
#include "QAPendingEvent.hpp"

#include <QClipboard>
#include <QGuiApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTcpSocket>
#include <QTimer>
#include <QMetaMethod>
#include <QJsonArray>
#include <QXmlStreamWriter>
#include <QXmlQuery>

#include "qpa/qwindowsysteminterface_p.h"

//void GenericEnginePlatform::findStrategyAs_name(QTcpSocket *socket, const QString &selector, bool multiple, QObject *parentItem)
//{
//    qDebug() << Q_FUNC_INFO << parentItem;
//}

GenericEnginePlatform::GenericEnginePlatform(QObject *parent)
    : IEnginePlatform(parent)
    , m_mouseEngine(new QAMouseEngine(this))
    , m_keyEngine(new QAKeyEngine(this))
{
    qDebug()
        << Q_FUNC_INFO;

    connect(m_mouseEngine, &QAMouseEngine::touchEvent, this, &GenericEnginePlatform::onTouchEvent);
    connect(m_mouseEngine, &QAMouseEngine::mouseEvent, this, &GenericEnginePlatform::onMouseEvent);
    connect(m_keyEngine, &QAKeyEngine::triggered, this, &GenericEnginePlatform::onKeyEvent);

    QTimer::singleShot(0, this, &GenericEnginePlatform::initialize);
}

void GenericEnginePlatform::socketReply(QTcpSocket *socket, const QVariant &value, int status)
{
    QJsonObject reply;
    reply.insert(QStringLiteral("status"), status);
    reply.insert(QStringLiteral("value"), QJsonValue::fromVariant(value));

    const QByteArray data = QJsonDocument(reply).toJson(QJsonDocument::Compact);

    qDebug()
        << Q_FUNC_INFO
        << socket
        << data.size()
        << "Reply is:";
    qDebug().noquote()
        << data;

    socket->write(data);
    socket->flush();
}

void GenericEnginePlatform::elementReply(QTcpSocket *socket, QObjectList elements, bool multiple)
{
    QVariantList value;
    for (QObject *item : elements) {
        if (!item) {
            continue;
        }
        const QString uId = uniqueId(item);
        m_items.insert(uId, item);

        QVariantMap element;
        element.insert(QStringLiteral("ELEMENT"), uId);
        value.append(element);
    }

    if (value.isEmpty()) {
        socketReply(socket, QString());
    } else if (!multiple) {
        socketReply(socket, value.first());
    } else {
        socketReply(socket, value);
    }
}

void GenericEnginePlatform::findElement(QTcpSocket *socket, const QString &strategy, const QString &selector, bool multiple, QObject *item)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector << multiple << item;

    QString fixStrategy = strategy;
    fixStrategy = fixStrategy.remove(QChar(u' '));
    const QString methodName = QStringLiteral("findStrategy_%1").arg(fixStrategy);
    if (!QAEngine::metaInvoke(socket, this, methodName, {selector, multiple, QVariant::fromValue(item)})) {
        findByProperty(socket, fixStrategy, selector, multiple, item);
    }
}

void GenericEnginePlatform::findByProperty(QTcpSocket *socket, const QString &propertyName, const QVariant &propertyValue, bool multiple, QObject *parentItem)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << propertyName << propertyValue << multiple << parentItem;

    QObjectList items = findItemsByProperty(propertyName, propertyValue, parentItem);
    elementReply(socket, items, multiple);
}

QObject *GenericEnginePlatform::findItemByObjectName(const QString &objectName, QObject *parentItem)
{
    if (!parentItem) {
        parentItem = m_rootObject;
    }

    QList<QObject*> childItems = childrenList(parentItem);
    for (QObject *child : childItems) {
        if (child->objectName() == objectName) {
            return child;
        }
        QObject *item = findItemByObjectName(objectName, child);
        if (item) {
            return item;
        }
    }
    return nullptr;
}

QObjectList GenericEnginePlatform::findItemsByClassName(const QString &className, QObject *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << className << parentItem;

    QObjectList items;

    if (!parentItem) {
        parentItem = m_rootObject;
    }

    for (QObject *child : childrenList(parentItem)) {
        if (getClassName(child) == className) {
            items.append(child);
        }
        QObjectList recursiveItems = findItemsByClassName(className, child);
        items.append(recursiveItems);
    }
    return items;
}

QObjectList GenericEnginePlatform::findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QObject *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << propertyName << propertyValue << parentItem;

    QObjectList items;

    if (!parentItem) {
        parentItem = m_rootObject;
    }

    for (QObject *child : childrenList(parentItem)) {
        if (child->property(propertyName.toLatin1().constData()) == propertyValue) {
            items.append(child);
        }
        QObjectList recursiveItems = findItemsByProperty(propertyName, propertyValue, child);
        items.append(recursiveItems);
    }
    return items;
}

QObjectList GenericEnginePlatform::findItemsByText(const QString &text, bool partial, QObject *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << text << partial << parentItem;

    QObjectList items;

    if (!parentItem) {
        parentItem = m_rootObject;
    }

    for (QObject *child : childrenList(parentItem)) {
        const QString &itemText = getText(child);
        if ((partial && itemText.contains(text)) || (!partial && itemText == text)) {
            items.append(child);
        }
        QObjectList recursiveItems = findItemsByText(text, partial, child);
        items.append(recursiveItems);
    }
    return items;
}

QObjectList GenericEnginePlatform::findItemsByXpath(const QString &xpath, QObject *parentItem)
{
    QObjectList items;

    if (!parentItem) {
        return items;
    }

    QString out;
    QXmlStreamWriter writer(&out);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    recursiveDumpXml(&writer, parentItem, 0);
    writer.writeEndDocument();

    QXmlQuery query;
    query.setFocus(out);
    query.setQuery(xpath);

    if (!query.isValid()) {
        return items;
    }
    QString tempString;
    query.evaluateTo(&tempString);

    if (tempString.trimmed().isEmpty()) {
        return items;
    }

    const QString resultData = QLatin1String("<results>") + tempString + QLatin1String("</results>");
    QXmlStreamReader reader(resultData);
    reader.readNextStartElement();
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            const QString elementId = reader.attributes().value(QStringLiteral("id")).toString();
            const QString address = elementId.section(QChar(u'x'), -1);
            const uint integer = address.toUInt(NULL, 16);
            QObject *item = reinterpret_cast<QObject*>(integer);
            items.append(item);
            reader.skipCurrentElement();
        }
    }

    return items;
}

QObjectList GenericEnginePlatform::filterVisibleItems(QObjectList items)
{
    qWarning()
        << Q_FUNC_INFO
        << items;

    QObjectList result;
    for (QObject *item : items) {
        if (!isItemVisible(item)) {
            continue;
        }
        result.append(item);
    }
    return result;
}

bool GenericEnginePlatform::containsObject(const QString &elementId)
{
    return m_items.contains(elementId);
}

QObject *GenericEnginePlatform::getObject(const QString &elementId)
{
    return m_items.value(elementId);
}

QString GenericEnginePlatform::getText(QObject *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    static const char *textProperties[] = {
        "label",
        "title",
        "description",
        "placeholderText",
        "text",
        "value",
        "name",
        "toolTip",
    };

    for (const char *textProperty : textProperties) {
        if (item->metaObject()->indexOfProperty(textProperty) > 0) {
            const QString text = item->property(textProperty).toString();
            if (!text.isEmpty()) {
                return text;
            }
        }
    }

    return QString();
}

QRect GenericEnginePlatform::getGeometry(QObject *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    return QRect(getPosition(item), getSize(item));
}

QRect GenericEnginePlatform::getAbsGeometry(QObject *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    return QRect(getAbsPosition(item), getSize(item));
}

QJsonObject GenericEnginePlatform::dumpObject(QObject *item, int depth)
{
    QJsonObject object;

    const QString className = getClassName(item);
    object.insert(QStringLiteral("classname"), QJsonValue(className));

    const QString id = uniqueId(item);
    object.insert(QStringLiteral("id"), QJsonValue(id));

    auto mo = item->metaObject();
    do {
        const QString moClassName = QString::fromLatin1(mo->className());
        std::vector<std::pair<QString, QVariant> > v;
        v.reserve(mo->propertyCount() - mo->propertyOffset());
        for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
            const QString propertyName = QString::fromLatin1(mo->property(i).name());
            if (m_blacklistedProperties.contains(moClassName)
                    && m_blacklistedProperties.value(moClassName).contains(propertyName)) {
                qDebug()
                    << "Found blacklisted:"
                    << moClassName << propertyName;
                continue;
            }

            v.emplace_back(propertyName, mo->property(i).read(item));
        }
        std::sort(v.begin(), v.end());
        for (auto &i : v) {
            if (!object.contains(i.first)
                    && i.second.canConvert<QString>()) {
                object.insert(i.first, QJsonValue::fromVariant(i.second));
            }
        }
    } while ((mo = mo->superClass()));

    const QRect rect = getGeometry(item);
    object.insert(QStringLiteral("width"), QJsonValue(rect.width()));
    object.insert(QStringLiteral("height"), QJsonValue(rect.height()));
    object.insert(QStringLiteral("x"), QJsonValue(rect.x()));
    object.insert(QStringLiteral("y"), QJsonValue(rect.y()));
    object.insert(QStringLiteral("depth"), QJsonValue(depth));

    const QPoint abs = getAbsPosition(item);
    object.insert(QStringLiteral("abs_x"), QJsonValue(abs.x()));
    object.insert(QStringLiteral("abs_y"), QJsonValue(abs.y()));

    object.insert(QStringLiteral("objectName"), QJsonValue(item->objectName()));
    object.insert(QStringLiteral("enabled"), QJsonValue(isItemEnabled(item)));
    object.insert(QStringLiteral("visible"), QJsonValue(isItemVisible(item)));

    object.insert(QStringLiteral("mainTextProperty"), getText(item));

    return object;
}

QJsonObject GenericEnginePlatform::recursiveDumpTree(QObject *rootItem, int depth)
{
    QJsonObject object = dumpObject(rootItem, depth);
    QJsonArray childArray;

    int z = 0;
    for (QObject *child : childrenList(rootItem)) {
        QJsonObject childObject = recursiveDumpTree(child, ++z);
        childArray.append(QJsonValue(childObject));
    }
    object.insert(QStringLiteral("children"), QJsonValue(childArray));

    return object;
}

void GenericEnginePlatform::recursiveDumpXml(QXmlStreamWriter *writer, QObject *rootItem, int depth)
{
    const QJsonObject object = dumpObject(rootItem, depth);
    writer->writeStartElement(object.value(QStringLiteral("classname")).toString());
    for (auto i = object.constBegin(), objEnd = object.constEnd(); i != objEnd; ++i) {
        const QJsonValue& val = *i;
        const QString &name = i.key();

        writer->writeAttribute(name, val.toVariant().toString());
    }
    if (object.contains(QStringLiteral("mainTextProperty"))) {
        writer->writeCharacters(object.value(QStringLiteral("mainTextProperty")).toString());
    }

    int z = 0;
    for (QObject *child : childrenList(rootItem)) {
        recursiveDumpXml(writer, child, ++z);
    }

    writer->writeEndElement();
}

void GenericEnginePlatform::clickItem(QObject *item)
{
    const QPoint itemAbs = getAbsPosition(item);
    const QSize size = getSize(item);
    qWarning()
        << Q_FUNC_INFO
        << item << itemAbs << size;

    clickPoint(itemAbs.x() + size.width() / 2, itemAbs.y() + size.height() / 2);
}

QString GenericEnginePlatform::getClassName(QObject *item)
{
    return QString::fromLatin1(item->metaObject()->className()).section(QChar(u'_'), 0, 0).section(QChar(u':'), -1);
}

QString GenericEnginePlatform::uniqueId(QObject *item)
{
    return QStringLiteral("%1_0x%2").arg(getClassName(item))
            .arg(reinterpret_cast<quintptr>(item),
                 QT_POINTER_SIZE * 2, 16, QLatin1Char('0'));
}

void GenericEnginePlatform::setProperty(QTcpSocket *socket, const QString &property, const QVariant &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << property << value << elementId;

    QObject *item = getObject(elementId);
    if (item) {
        item->setProperty(property.toLatin1().constData(), value);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void GenericEnginePlatform::clickPoint(int posx, int posy)
{
    qWarning()
        << Q_FUNC_INFO
        << posx << posy;

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(50);
    connect(m_mouseEngine->click(QPointF(posx, posy)),
            &QAPendingEvent::completed, &timer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();
}

void GenericEnginePlatform::clickPoint(const QPoint &pos)
{
    clickPoint(pos.x(), pos.y());
}

void GenericEnginePlatform::pressAndHold(int posx, int posy, int delay)
{
    qWarning()
        << Q_FUNC_INFO
        << posx << posy << delay;

    QEventLoop loop;
    connect(m_mouseEngine->pressAndHold(QPointF(posx, posy), delay),
            &QAPendingEvent::completed, &loop, &QEventLoop::quit);
    loop.exec();
}

void GenericEnginePlatform::mouseMove(int startx, int starty, int stopx, int stopy)
{
    qWarning()
        << Q_FUNC_INFO
        << startx << starty << stopx << stopy;

    QEventLoop loop;
    QTimer timer;
    timer.setInterval(800);
    timer.setSingleShot(true);
    connect(m_mouseEngine->move(
                QPointF(startx, starty),
                QPointF(stopx, stopy)),
            &QAPendingEvent::completed, &timer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();
}

void GenericEnginePlatform::mouseDrag(int startx, int starty, int stopx, int stopy, int delay)
{
    qWarning()
        << Q_FUNC_INFO
        << startx << starty << stopx << stopy << delay;

    QEventLoop loop;
    QTimer timer;
    timer.setInterval(800);
    timer.setSingleShot(true);
    connect(m_mouseEngine->drag(
                QPointF(startx, starty),
                QPointF(stopx, stopy),
                delay),
            &QAPendingEvent::completed, &timer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();
}

void GenericEnginePlatform::processTouchActionList(const QVariant &actionListArg)
{
    int startX = 0;
    int startY = 0;
    int endX = 0;
    int endY = 0;
    int delay = 800;

    const QVariantList actions = actionListArg.toList();
    for (const QVariant &actionArg : actions) {
        const QVariantMap action = actionArg.toMap();
        const QString actionName = action.value(QStringLiteral("action")).toString();
        const QVariantMap options = action.value(QStringLiteral("options")).toMap();

        if (actionName == QLatin1String("wait")) {
            delay = options.value(QStringLiteral("ms")).toInt();
        } else if (actionName == QLatin1String("tap")) {
            const int tapX = options.value(QStringLiteral("x")).toInt();
            const int tapY = options.value(QStringLiteral("y")).toInt();
            clickPoint(tapX, tapY);
        } else if (actionName == QLatin1String("press")) {
            startX = options.value(QStringLiteral("x")).toInt();
            startY = options.value(QStringLiteral("y")).toInt();
        } else if (actionName == QLatin1String("moveTo")) {
            endX = options.value(QStringLiteral("x")).toInt();
            endY = options.value(QStringLiteral("y")).toInt();
        } else if (actionName == QLatin1String("release")) {
            mouseDrag(startX, startY, endX, endY, delay);
        } else if (actionName == QLatin1String("longPress")) {
            const QString elementId = options.value(QStringLiteral("element")).toString();
            if (!containsObject(elementId)) {
                continue;
            }
            pressAndHoldItem(getObject(elementId), delay);
        }
    }

}

void GenericEnginePlatform::waitForPropertyChange(QObject *item, const QString &propertyName, const QVariant &value, int timeout)
{
    qWarning()
        << Q_FUNC_INFO
        << item << propertyName << value << timeout;

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
        qWarning()
            << Q_FUNC_INFO
            << item << "property" << propertyName << "have on notifySignal!";
        return;
    }
    QEventLoop loop;
    QTimer timer;
    item->setProperty("WaitForPropertyChangeEventLoop", QVariant::fromValue(&loop));
    item->setProperty("WaitForPropertyChangePropertyName", propertyName);
    item->setProperty("WaitForPropertyChangePropertyValue", value);
    const QMetaMethod propertyChanged = metaObject()->method(metaObject()->indexOfSlot("onPropertyChanged()"));
    connect(item, prop.notifySignal(), this, propertyChanged);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(timeout);
    loop.exec();
    disconnect(item, prop.notifySignal(), this, propertyChanged);
}

void GenericEnginePlatform::execute(QTcpSocket *socket, const QString &methodName, const QVariantList &params)
{
    bool handled = false;
    bool success = QAEngine::metaInvoke(socket, this, methodName, params, &handled);

    if (!handled || !success) {
        qWarning()
            << Q_FUNC_INFO
            << methodName << "not handled!";
        socketReply(socket, QString());
    }
}

void GenericEnginePlatform::onPropertyChanged()
{
    QObject *item = sender();
    QEventLoop *loop = item->property("WaitForPropertyChangeEventLoop").value<QEventLoop*>();
    if (!loop) {
        return;
    }
    const QString propertyName = item->property("WaitForPropertyChangePropertyName").toString();
    const QVariant propertyValue = item->property("WaitForPropertyChangePropertyValue");
    if (!propertyValue.isValid()) {
        loop->quit();
    }
    const QVariant property = item->property(propertyName.toLatin1().constData());
    if (property == propertyValue) {
        loop->quit();
    }
}

void GenericEnginePlatform::onTouchEvent(const QTouchEvent &event)
{
    QWindowSystemInterface::handleTouchEvent(
        m_rootWindow,
        event.timestamp(),
        event.device(),
        QWindowSystemInterfacePrivate::toNativeTouchPoints(
            event.touchPoints(),
            m_rootWindow));
}

void GenericEnginePlatform::onMouseEvent(const QMouseEvent &event)
{
    QWindowSystemInterface::handleMouseEvent(
        m_rootWindow,
        event.timestamp(),
        event.localPos(),
        event.globalPos(),
        event.buttons(),
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
        event.button(),
        event.type(),
#endif
        Qt::NoModifier,
        Qt::MouseEventNotSynthesized);
}

void GenericEnginePlatform::onKeyEvent(QKeyEvent *event)
{
    QWindowSystemInterface::handleKeyEvent(
        m_rootWindow,
        event->type(),
        event->key(),
        event->modifiers(),
        event->text());
}

void GenericEnginePlatform::activateAppCommand(QTcpSocket *socket, const QString &appName)
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

    m_rootWindow->raise();
    m_rootWindow->requestActivate();
    m_rootWindow->setWindowState(Qt::WindowState::WindowActive);

    socketReply(socket, QString());
}

void GenericEnginePlatform::closeAppCommand(QTcpSocket *socket, const QString &appName)
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

    socketReply(socket, QString());
    qApp->quit();
}

void GenericEnginePlatform::queryAppStateCommand(QTcpSocket *socket, const QString &appName)
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

void GenericEnginePlatform::backgroundCommand(QTcpSocket *socket, double seconds)
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

    m_rootWindow->showMinimized();
    m_rootWindow->lower();
    if (seconds > 0) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(seconds * 1000);
        loop.exec();
        m_rootWindow->raise();
        m_rootWindow->requestActivate();
        m_rootWindow->setWindowState(Qt::WindowState::WindowActive);
    }

    socketReply(socket, QString());
}

void GenericEnginePlatform::getClipboardCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;

    socketReply(socket, QString::fromUtf8(qGuiApp->clipboard()->text().toUtf8().toBase64()));
}

void GenericEnginePlatform::setClipboardCommand(QTcpSocket *socket, const QString &content)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << content;

    qGuiApp->clipboard()->setText(content);
    socketReply(socket, QString());
}

void GenericEnginePlatform::findElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector;

    findElement(socket, strategy, selector);
}

void GenericEnginePlatform::findElementsCommand(QTcpSocket *socket, const QString &strategy, const QString &selector)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector;

    findElement(socket, strategy, selector, true);
}

void GenericEnginePlatform::findElementFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector << elementId;

    findElement(socket, strategy, selector, false, getObject(elementId));
}

void GenericEnginePlatform::findElementsFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector << elementId;

    findElement(socket, strategy, selector, true, getObject(elementId));
}

void GenericEnginePlatform::getLocationCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QObject *item = getObject(elementId);
    if (item) {
        QJsonObject reply;
        const QRect geometry = getAbsGeometry(item);
        reply.insert(QStringLiteral("centerx"), geometry.center().x());
        reply.insert(QStringLiteral("centery"), geometry.center().y());
        reply.insert(QStringLiteral("x"), geometry.x());
        reply.insert(QStringLiteral("y"), geometry.y());
        reply.insert(QStringLiteral("width"), geometry.width());
        reply.insert(QStringLiteral("height"), geometry.height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void GenericEnginePlatform::getLocationInViewCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QObject *item = getObject(elementId);
    if (item) {
        QJsonObject reply;
        const QRect geometry = getGeometry(item);
        reply.insert(QStringLiteral("centerx"), geometry.center().x());
        reply.insert(QStringLiteral("centery"), geometry.center().y());
        reply.insert(QStringLiteral("x"), geometry.x());
        reply.insert(QStringLiteral("y"), geometry.y());
        reply.insert(QStringLiteral("width"), geometry.width());
        reply.insert(QStringLiteral("height"), geometry.height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void GenericEnginePlatform::getAttributeCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << attribute << elementId;

    QObject *item = getObject(elementId);
    if (item) {
        const QVariant reply = item->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void GenericEnginePlatform::getPropertyCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << attribute << elementId;

    QObject *item = getObject(elementId);
    if (item) {
        const QVariant reply = item->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void GenericEnginePlatform::getTextCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QObject *item = getObject(elementId);
    if (item) {
        socketReply(socket, getText(item));
    } else {
        socketReply(socket, QString());
    }
}

void GenericEnginePlatform::getElementScreenshotCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QObject *item = getObject(elementId);
    if (item) {
        grabScreenshot(socket, item, true);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void GenericEnginePlatform::getScreenshotCommand(QTcpSocket *socket)
{
    qWarning()
        << Q_FUNC_INFO
        << socket;

    grabScreenshot(socket, m_rootObject, true);
}

void GenericEnginePlatform::elementEnabledCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("enabled"), elementId);
}

void GenericEnginePlatform::elementDisplayedCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("visible"), elementId);
}

void GenericEnginePlatform::elementSelectedCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("checked"), elementId);
}

void GenericEnginePlatform::getSizeCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QObject *item = getObject(elementId);
    if (item) {
        QJsonObject reply;
        const QSize size = getSize(item);
        reply.insert(QStringLiteral("width"), size.width());
        reply.insert(QStringLiteral("height"), size.height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void GenericEnginePlatform::setValueImmediateCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    QStringList text;
    for (const QVariant &val : value) {
        text.append(val.toString());
    }

    setProperty(socket, QStringLiteral("text"), text.join(QString()), elementId);
}

void GenericEnginePlatform::replaceValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    setValueImmediateCommand(socket, value, elementId);
}

void GenericEnginePlatform::setValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    setValueImmediateCommand(socket, value, elementId);
}

void GenericEnginePlatform::clickCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QObject *item = getObject(elementId);
    if (item) {
        clickItem(item);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void GenericEnginePlatform::clearCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    setProperty(socket, QStringLiteral("text"), QString(), elementId);
}

void GenericEnginePlatform::submitCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    m_keyEngine->pressEnter(1);
    socketReply(socket, QString());
}

void GenericEnginePlatform::getPageSourceCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::backCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::forwardCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getOrientationCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::setOrientationCommand(QTcpSocket *socket, const QString &orientation)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << orientation;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::hideKeyboardCommand(QTcpSocket *socket, const QString &strategy, const QString &key, double keyCode, const QString &keyName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << strategy << key << keyCode << keyName;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getCurrentActivityCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::implicitWaitCommand(QTcpSocket *socket, double msecs)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << msecs;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::activeCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getAlertTextCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::isKeyboardShownCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::activateIMEEngineCommand(QTcpSocket *socket, const QVariant &engine)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << engine;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::availableIMEEnginesCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getActiveIMEEngineCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::deactivateIMEEngineCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::isIMEActivatedCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::keyeventCommand(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &sessionIDArg, const QVariant &flagsArg)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << keycodeArg << metaState << sessionIDArg << flagsArg;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::longPressKeyCodeCommand(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << keycodeArg << metaState << flagsArg;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::pressKeyCodeCommand(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << keycodeArg << metaState << flagsArg;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::executeCommand(QTcpSocket *socket, const QString &command, const QVariantList &params)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << command << params;

    const QString fixCommand = QString(command).replace(QChar(':'), QChar('_'));
    const QString methodName = QStringLiteral("executeCommand_%1").arg(fixCommand);
    execute(socket, methodName, params);
}

void GenericEnginePlatform::executeAsyncCommand(QTcpSocket *socket, const QString &command, const QVariantList &params)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << command << params;

    const QString fixCommand = QString(command).replace(QChar(':'), QChar('_'));
    const QString methodName = QStringLiteral("executeCommand_%1").arg(fixCommand); // executeCommandAsync_ ?
    execute(socket, methodName, params);
}

void GenericEnginePlatform::performTouchCommand(QTcpSocket *socket, const QVariant &paramsArg)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << paramsArg;

    processTouchActionList(paramsArg);
    socketReply(socket, QString());

}

void GenericEnginePlatform::findStrategy_id(QTcpSocket *socket, const QString &selector, bool multiple, QObject *parentItem)
{
    QObject *item = findItemByObjectName(selector, parentItem);
    qDebug()
        << Q_FUNC_INFO
        << selector << multiple << item;
    elementReply(socket, {item}, multiple);
}

void GenericEnginePlatform::findStrategy_objectName(QTcpSocket *socket, const QString &selector, bool multiple, QObject *parentItem)
{
    findStrategy_id(socket, selector, multiple, parentItem);
}

void GenericEnginePlatform::findStrategy_classname(QTcpSocket *socket, const QString &selector, bool multiple, QObject *parentItem)
{
    QObjectList items = findItemsByClassName(selector, parentItem);
    qDebug()
        << Q_FUNC_INFO
        << selector << multiple << items;
    elementReply(socket, items, multiple);
}

void GenericEnginePlatform::findStrategy_name(QTcpSocket *socket, const QString &selector, bool multiple, QObject *parentItem)
{
    QObjectList items = findItemsByText(selector, false, parentItem);
    qDebug()
        << Q_FUNC_INFO
        << selector << multiple << items;
    elementReply(socket, items, multiple);
}

void GenericEnginePlatform::findStrategy_parent(QTcpSocket *socket, const QString &selector, bool multiple, QObject *parentItem)
{
    const int depth = selector.toInt();
    QObject *pItem = getParent(parentItem);
    for (int i = 0; i < depth; i++) {
        if (!pItem) {
            break;
        }
        pItem = getParent(pItem);
    }
    const QObjectList items = {pItem};
    qDebug()
        << Q_FUNC_INFO
        << selector << multiple << items;
    elementReply(socket, items, multiple);
}

void GenericEnginePlatform::findStrategy_xpath(QTcpSocket *socket, const QString &selector, bool multiple, QObject *parentItem)
{
    QObjectList items = findItemsByXpath(selector, parentItem);
    qDebug() << Q_FUNC_INFO << selector << multiple << items;
    elementReply(socket, items, multiple);
}

void GenericEnginePlatform::executeCommand_app_dumpTree(QTcpSocket *socket)
{
    qWarning()
        << Q_FUNC_INFO
        << socket;

    QJsonObject reply = recursiveDumpTree(m_rootObject);
    socketReply(socket, qCompress(QJsonDocument(reply).toJson(QJsonDocument::Compact), 9).toBase64());
}

void GenericEnginePlatform::executeCommand_app_setAttribute(QTcpSocket *socket, const QString &elementId, const QString &attribute, const QVariant &value)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId << attribute << value;

    setProperty(socket, attribute, value, elementId);
}

void GenericEnginePlatform::executeCommand_app_waitForPropertyChange(QTcpSocket *socket, const QString &elementId, const QString &propertyName, const QVariant &value, double timeout)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId << propertyName << value << timeout;

    QObject *item = getObject(elementId);
    if (item) {
        waitForPropertyChange(item, propertyName, value, timeout);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}
