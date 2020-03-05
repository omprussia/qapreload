#include "QAEngine.hpp"
#include "WidgetsEnginePlatform.hpp"
#include "QAMouseEngine.hpp"
#include "QAKeyEngine.hpp"

#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaMethod>
#include <QMetaObject>
#include <QTimer>
#include <QWidget>
#include <QWindow>
#include <QXmlQuery>
#include <QXmlStreamWriter>

#include <private/qwindow_p.h>
#include <private/qwidget_p.h>

WidgetsEnginePlatform::WidgetsEnginePlatform(QObject *parent)
    : GenericEnginePlatform(parent)
{

}

QWidget *WidgetsEnginePlatform::findItemByObjectName(const QString &objectName, QWidget *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << objectName << parentItem;

    if (!parentItem) {
        parentItem = m_rootWidget;
    }

    for (QWidget *child : parentItem->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        if (child->objectName() == objectName) {
            return child;
        }
        QWidget *item = findItemByObjectName(objectName, child);
        if (item) {
            return item;
        }
    }
    return nullptr;
}

QVariantList WidgetsEnginePlatform::findItemsByClassName(const QString &className, QWidget *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << className << parentItem;

    QVariantList items;

    if (!parentItem) {
        parentItem = m_rootWidget;
    }

    for (QWidget *child : parentItem->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        if (getClassName(child) == className) {
            items.append(QVariant::fromValue(child));
        }
        QVariantList recursiveItems = findItemsByClassName(className, child);
        items.append(recursiveItems);
    }
    return items;
}

QVariantList WidgetsEnginePlatform::findItemsByProperty(const QString &propertyName, const QVariant &propertyValue, QWidget *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << propertyName << propertyValue << parentItem;

    QVariantList items;

    if (!parentItem) {
        parentItem = m_rootWidget;
    }

    for (QWidget *child : parentItem->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        if (child->property(propertyName.toLatin1().constData()) == propertyValue) {
            items.append(QVariant::fromValue(child));
        }
        QVariantList recursiveItems = findItemsByProperty(propertyName, propertyValue, child);
        items.append(recursiveItems);
    }
    return items;
}

QVariantList WidgetsEnginePlatform::findItemsByText(const QString &text, bool partial, QWidget *parentItem)
{
    qWarning()
        << Q_FUNC_INFO
        << text << partial << parentItem;

    QVariantList items;

    if (!parentItem) {
        parentItem = m_rootWidget;
    }

    for (QWidget *child : parentItem->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        const QString &itemText = getText(child);
        if ((partial && itemText.contains(text)) || (!partial && itemText == text)) {
            items.append(QVariant::fromValue(child));
        }
        QVariantList recursiveItems = findItemsByText(text, partial, child);
        items.append(recursiveItems);
    }
    return items;
}

QVariantList WidgetsEnginePlatform::findItemsByXpath(const QString &xpath, QWidget *parentItem)
{
    QVariantList items;

    if (!parentItem) {
        parentItem = m_rootWidget;
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
            QWidget *item = reinterpret_cast<QWidget*>(integer);
            items.append(QVariant::fromValue(item));
            reader.skipCurrentElement();
        }
    }

    return items;
}

void WidgetsEnginePlatform::findByProperty(QTcpSocket *socket, const QString &propertyName, const QVariant &propertyValue, bool multiple, QWidget *parentItem)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << propertyName << propertyValue << multiple << parentItem;

    QVariantList items = findItemsByProperty(propertyName, propertyValue, parentItem);
    elementReply(socket, items, multiple);
}

QVariantList WidgetsEnginePlatform::filterVisibleItems(const QVariantList &items)
{
    qWarning()
        << Q_FUNC_INFO
        << items;

    QVariantList result;
    for (const QVariant &itemVariant : items) {
        QWidget *item = itemVariant.value<QWidget*>();
        if (!item || !item->isVisible()) {
            continue;
        }
        result.append(itemVariant);
    }
    return result;
}

void WidgetsEnginePlatform::initialize()
{
    m_rootWindow = QApplication::focusWindow();
    connect(qApp, &QApplication::focusWindowChanged, this, &WidgetsEnginePlatform::onFocusWindowChanged);
    onFocusWindowChanged(m_rootWindow);
}

void WidgetsEnginePlatform::onFocusWindowChanged(QWindow *window)
{
    qDebug()
        << Q_FUNC_INFO
        << window << qApp->activeWindow();

    if (!window || window == m_rootWindow) {
        return;
    }

    m_rootWindow = window;
    m_rootWidget = qApp->activeWindow();
    m_rootObject = m_rootWidget;

    emit ready();
}

void WidgetsEnginePlatform::findElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector;

    findElement(socket, strategy, selector);
}

void WidgetsEnginePlatform::findElementsCommand(QTcpSocket *socket, const QString &strategy, const QString &selector)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector;

    findElement(socket, strategy, selector, true);
}

void WidgetsEnginePlatform::findElementFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector << elementId;

    findElement(socket, strategy, selector, false, getItem<QWidget*>(elementId));
}

void WidgetsEnginePlatform::findElementsFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector << elementId;

    findElement(socket, strategy, selector, true, getItem<QWidget*>(elementId));
}

void WidgetsEnginePlatform::getLocationCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem<QWidget*>(elementId);
    if (item) {
        QJsonObject reply;
        const QPointF absPoint = getAbsPosition(item);
        reply.insert(QStringLiteral("centerx"), absPoint.x() + item->width() / 2);
        reply.insert(QStringLiteral("centery"), absPoint.y() + item->height() / 2);
        reply.insert(QStringLiteral("x"), absPoint.x());
        reply.insert(QStringLiteral("y"), absPoint.y());
        reply.insert(QStringLiteral("width"),item->width());
        reply.insert(QStringLiteral("height"), item->height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void WidgetsEnginePlatform::getLocationInViewCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem<QWidget*>(elementId);
    if (item) {
        QJsonObject reply;
        reply.insert(QStringLiteral("centerx"), item->x() + item->width() / 2);
        reply.insert(QStringLiteral("centery"), item->y() + item->height() / 2);
        reply.insert(QStringLiteral("x"), item->x());
        reply.insert(QStringLiteral("y"), item->y());
        reply.insert(QStringLiteral("width"),item->width());
        reply.insert(QStringLiteral("height"), item->height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void WidgetsEnginePlatform::getAttributeCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << attribute << elementId;

    QWidget *item = getItem<QWidget*>(elementId);
    if (item) {
        const QVariant reply = item->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void WidgetsEnginePlatform::getPropertyCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << attribute << elementId;

    QWidget *item = getItem<QWidget*>(elementId);
    if (item) {
        const QVariant reply = item->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void WidgetsEnginePlatform::getTextCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem<QWidget*>(elementId);
    if (item) {
        socketReply(socket, getText(item));
    } else {
        socketReply(socket, QString());
    }
}

void WidgetsEnginePlatform::getElementScreenshotCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem<QWidget*>(elementId);
    if (item) {
        grabScreenshot(socket, item, true);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void WidgetsEnginePlatform::getScreenshotCommand(QTcpSocket *socket)
{
    qWarning()
        << Q_FUNC_INFO
        << socket;

    grabScreenshot(socket, m_rootWidget, true);
}

void WidgetsEnginePlatform::elementEnabledCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("enabled"), elementId);
}

void WidgetsEnginePlatform::elementDisplayedCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("visible"), elementId);
}

void WidgetsEnginePlatform::elementSelectedCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    getAttributeCommand(socket, QStringLiteral("checked"), elementId);
}

void WidgetsEnginePlatform::getSizeCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem<QWidget*>(elementId);
    if (item) {
        QJsonObject reply;
        reply.insert(QStringLiteral("width"), item->width());
        reply.insert(QStringLiteral("height"), item->height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void WidgetsEnginePlatform::setValueImmediateCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
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

void WidgetsEnginePlatform::replaceValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    setValueImmediateCommand(socket, value, elementId);
}

void WidgetsEnginePlatform::setValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << value << elementId;

    setValueImmediateCommand(socket, value, elementId);
}

void WidgetsEnginePlatform::clickCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    QWidget *item = getItem<QWidget*>(elementId);
    if (item) {
        clickItem(item);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void WidgetsEnginePlatform::clearCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    setProperty(socket, QStringLiteral("text"), QString(), elementId);
}

void WidgetsEnginePlatform::submitCommand(QTcpSocket *socket, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << elementId;

    m_keyEngine->pressEnter(1);
    socketReply(socket, QString());
}

void WidgetsEnginePlatform::getPageSourceCommand(QTcpSocket *socket)
{
    qWarning()
        << Q_FUNC_INFO
        << socket;

    // TODO

    socketReply(socket, QString());
}

void WidgetsEnginePlatform::executeCommand_app_dumpTree(QTcpSocket *socket)
{
    qWarning()
        << Q_FUNC_INFO
        << socket;

    QJsonObject reply = recursiveDumpTree(m_rootWidget);
    socketReply(socket, QJsonDocument(reply).toJson(QJsonDocument::Compact));
}

void WidgetsEnginePlatform::findStrategy_id(QTcpSocket *socket, const QString &selector, bool multiple, QWidget *parentItem)
{
    QWidget *item = findItemByObjectName(selector, parentItem);
    qDebug()
        << Q_FUNC_INFO
        << selector << multiple << item;
    elementReply(socket, {QVariant::fromValue(item)}, multiple);
}

void WidgetsEnginePlatform::findStrategy_objectName(QTcpSocket *socket, const QString &selector, bool multiple, QWidget *parentItem)
{
    findStrategy_id(socket, selector, multiple, parentItem);
}

void WidgetsEnginePlatform::findStrategy_classname(QTcpSocket *socket, const QString &selector, bool multiple, QWidget *parentItem)
{
    QVariantList items = findItemsByClassName(selector, parentItem);
    qDebug()
        << Q_FUNC_INFO
        << selector << multiple << items;
    elementReply(socket, items, multiple);
}

void WidgetsEnginePlatform::findStrategy_parent(QTcpSocket *socket, const QString &selector, bool multiple, QWidget *parentItem)
{
    QVariantList items = findItemsByText(selector, false, parentItem);
    qDebug()
        << Q_FUNC_INFO
        << selector << multiple << items;
    elementReply(socket, items, multiple);
}

void WidgetsEnginePlatform::findStrategy_xpath(QTcpSocket *socket, const QString &selector, bool multiple, QWidget *parentItem)
{
    const int depth = selector.toInt();
    QWidget *pItem = parentItem->parentWidget();
    for (int i = 0; i < depth; i++) {
        if (!pItem) {
            break;
        }
        pItem = pItem->parentWidget();
    }
    const QVariantList items = {QVariant::fromValue(pItem)};
    qDebug()
        << Q_FUNC_INFO
        << selector << multiple << items;
    elementReply(socket, items, multiple);
}

void WidgetsEnginePlatform::findStrategy_name(QTcpSocket *socket, const QString &selector, bool multiple, QWidget *parentItem)
{
    QVariantList items = findItemsByText(selector, false, parentItem);
    qDebug()
        << Q_FUNC_INFO
        << selector << multiple << items;
    elementReply(socket, items, multiple);
}

void WidgetsEnginePlatform::recursiveDumpXml(QXmlStreamWriter *writer, QWidget *rootItem, int depth)
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
    for (QWidget *child : rootItem->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
        recursiveDumpXml(writer, child, ++z);
    }

    writer->writeEndElement();
}

QJsonObject WidgetsEnginePlatform::recursiveDumpTree(QWidget *rootItem, int depth)
{
    QJsonObject object = dumpObject(rootItem, depth);
    QJsonArray childArray;

    int z = 0;
//    for (QWidget *child : rootItem->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly)) {
    for (QObject *obj : rootItem->children()) {
        QWidget *child = qobject_cast<QWidget*>(obj);
        if (!child) {
            continue;
        }
        QJsonObject childObject = recursiveDumpTree(child, ++z);
        childArray.append(QJsonValue(childObject));
    }
    object.insert(QStringLiteral("children"), QJsonValue(childArray));

    return object;
}

QJsonObject WidgetsEnginePlatform:: dumpObject(QWidget *item, int depth)
{
    QJsonObject object;

    const QString className = getClassName(item);
    object.insert(QStringLiteral("classname"), QJsonValue(className));

    const QString id = uniqueId(item);
    object.insert(QStringLiteral("id"), QJsonValue(id));

//    auto mo = item->metaObject();
//    do {
//      const QString moClassName = QString::fromLatin1(mo->className());
//      std::vector<std::pair<QString, QVariant> > v;
//      v.reserve(mo->propertyCount() - mo->propertyOffset());
//      for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
//          const QString propertyName = QString::fromLatin1(mo->property(i).name());
//          v.emplace_back(propertyName,
//                         mo->property(i).read(item));
//      }
//      std::sort(v.begin(), v.end());
//      for (auto &i : v) {
//          if (!object.contains(i.first)
//                  && i.second.canConvert<QString>()) {
//              object.insert(i.first, QJsonValue::fromVariant(i.second));
//          }
//      }
//    } while ((mo = mo->superClass()));

    const QRect rect(item->x(), item->y(), item->width(), item->height());
    object.insert(QStringLiteral("width"), QJsonValue(rect.width()));
    object.insert(QStringLiteral("height"), QJsonValue(rect.height()));
    object.insert(QStringLiteral("x"), QJsonValue(rect.x()));
    object.insert(QStringLiteral("y"), QJsonValue(rect.y()));
    object.insert(QStringLiteral("depth"), QJsonValue(depth));

    const QPoint abs = getAbsPosition(item);
    object.insert(QStringLiteral("abs_x"), QJsonValue(abs.x()));
    object.insert(QStringLiteral("abs_y"), QJsonValue(abs.y()));

    object.insert(QStringLiteral("objectName"), QJsonValue(item->objectName()));
    object.insert(QStringLiteral("enabled"), QJsonValue(item->isEnabled()));
    object.insert(QStringLiteral("visible"), QJsonValue(item->isVisible()));

    object.insert(QStringLiteral("mainTextProperty"), getText(item));

    return object;
}

QPoint WidgetsEnginePlatform::getAbsPosition(QWidget *item)
{
    qWarning()
        << Q_FUNC_INFO
        << item;

    if (item == m_rootWidget) {
        return QPoint();
    } else {
        QSize frameSize = item->window()->frameSize() - item->window()->size();
        return item->mapToGlobal(-m_rootWidget->pos()) - QPoint(frameSize.width(), frameSize.height());
    }
}

QString WidgetsEnginePlatform::getText(QWidget *item)
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
    };

    for (const char *textProperty : textProperties) {
        if (item->metaObject()->indexOfProperty(textProperty) > 0) {
            return item->property(textProperty).toString();
        }
    }

    return QString();
}

void WidgetsEnginePlatform::grabScreenshot(QTcpSocket *socket, QWidget *item, bool fillBackground)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << item << fillBackground;

    QByteArray arr;
    QBuffer buffer(&arr);
    QPixmap pix = item->grab();
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

void WidgetsEnginePlatform::findElement(QTcpSocket *socket, const QString &strategy, const QString &selector, bool multiple, QWidget *item)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << strategy << selector << multiple << item;

    QString fixStrategy = strategy;
    fixStrategy = fixStrategy.remove(QChar(u' '));
    const QString methodName = QStringLiteral("findStrategy_%1").arg(fixStrategy);
    if (!QAEngine::metaInvoke(socket, this, methodName, {selector, multiple, QVariant::fromValue(reinterpret_cast<QWidget*>(item))})) {
        findByProperty(socket, fixStrategy, selector, multiple, item);
    }
}

void WidgetsEnginePlatform::clickItem(QWidget *item)
{
    const QPoint itemAbs = getAbsPosition(item);
    qWarning()
        << Q_FUNC_INFO
        << item << itemAbs;

    clickPoint(itemAbs.x() + item->width() / 2, itemAbs.y() + item->height() / 2);
}

void WidgetsEnginePlatform::setProperty(QTcpSocket *socket, const QString &property, const QString &value, const QString &elementId)
{
    qWarning()
        << Q_FUNC_INFO
        << socket << property << value << elementId;

    QWidget *item = getItem<QWidget*>(elementId);
    if (item) {
        item->setProperty(property.toLatin1().constData(), value);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void WidgetsEnginePlatform::pressAndHoldItem(QObject *qitem, int delay)
{
    qWarning()
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
