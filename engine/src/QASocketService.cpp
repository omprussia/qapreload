#include "QAEngine.hpp"
#include "QAKeyEngine.hpp"
#include "SailfishTest.hpp"
#include "qaservice_adaptor.h"

#include <QPainter>
#include <QPixmap>
#include <QQuickItemGrabResult>

#include "QASocketService.hpp"
#include <QCoreApplication>

#include <private/qquickwindow_p.h>

static QASocketService *s_instance = nullptr;

QASocketService::QASocketService(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QTcpSocket*>();
}

QASocketService *QASocketService::instance()
{
    if (!s_instance) {
        s_instance = new QASocketService(qApp);
    }
    return s_instance;
}

void QASocketService::initialize()
{
    if (m_server) {
        return;
    } else {
        m_server = new QTcpServer(this);
        connect(m_server, &QTcpServer::newConnection, this, &QASocketService::newConnection);
        qWarning() << Q_FUNC_INFO << m_server->listen(QHostAddress::AnyIPv4);

        m_sailfishTest = new SailfishTest(m_server);
    }
}

void QASocketService::elementReply(QTcpSocket *socket, const QVariantList &elements, bool multiple)
{
    QVariantList value;
    for (const QVariant vitem : elements) {
        QQuickItem *item = vitem.value<QQuickItem*>();
        if (!item) {
            continue;
        }
        const QString uId = QAEngine::uniqueId(item);
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

void QASocketService::socketReply(QTcpSocket *socket, const QVariant &value, int status)
{
    QJsonObject reply;
    reply.insert(QStringLiteral("status"), status);
    reply.insert(QStringLiteral("value"), QJsonValue::fromVariant(value));

    const QByteArray data = QJsonDocument(reply).toJson(QJsonDocument::Compact);

    qWarning().noquote() << data;

    socket->write(data);
    socket->flush();
}

void QASocketService::grabScreenshot(QTcpSocket *socket, QQuickItem *item, bool fillBackground)
{
    QSharedPointer<QQuickItemGrabResult> grabber = item->grabToImage();

    connect(grabber.data(), &QQuickItemGrabResult::ready, [this, grabber, socket, fillBackground]() {
        QByteArray arr;
        QBuffer buffer(&arr);
        buffer.open(QIODevice::WriteOnly);
        if (fillBackground) {
            QPixmap pixmap(grabber->image().width(), grabber->image().height());
            QPainter painter(&pixmap);
            painter.fillRect(0, 0, pixmap.width(), pixmap.height(), Qt::black);
            painter.drawImage(0, 0, grabber->image());
            pixmap.save(&buffer, "PNG");
        } else {
            grabber->image().save(&buffer, "PNG");
        }
        socketReply(socket, arr.toBase64());
    });
}

quint16 QASocketService::serverPort()
{
    initialize();
    return m_server->serverPort();
}

bool QASocketService::isListening()
{
    return m_server && m_server->isListening();
}

void QASocketService::stopListen()
{
    if (!m_server || !m_server->isListening()) {
        return;
    }
    m_server->close();
    disconnect(m_server, 0, 0, 0);
    m_server->deleteLater();
    m_server = nullptr;
}

void QASocketService::newConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &QASocketService::readSocket);
    qDebug() << Q_FUNC_INFO << "New connection from" << socket->peerAddress().toString() << socket->peerPort();
}

void QASocketService::readSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    const QByteArray requestData = socket->readAll();
    qDebug().noquote() << requestData;
    QJsonParseError error;
    QJsonDocument json = QJsonDocument::fromJson(requestData, &error);
    if (error.error != QJsonParseError::NoError) {
        return;
    }

    QJsonObject object = json.object();
    if (!object.contains(QStringLiteral("cmd"))) {
        return;
    }

    if (object.value(QStringLiteral("cmd")).toVariant().toString() != QStringLiteral("action")) {
        return;
    }

    if (!object.contains(QStringLiteral("action"))) {
        return;
    }

    const QString action = object.value(QStringLiteral("action")).toVariant().toString();
    const QVariantList params = object.value(QStringLiteral("params")).toVariant().toList();

    QGenericArgument arguments[9] = { QGenericArgument() };
    for (int i = 0; i < params.length(); i++) {
        arguments[i] = Q_ARG(QVariant, params[i]);
    }

    QMetaObject::invokeMethod(this,
                              QStringLiteral("%1Bootstrap").arg(action).toLatin1().constData(),
                              Qt::QueuedConnection,
                              Q_ARG(QTcpSocket*, socket),
                              arguments[0],
                              arguments[1],
                              arguments[2],
                              arguments[3],
                              arguments[4],
                              arguments[5],
                              arguments[6],
                              arguments[7],
                              arguments[8]);
}

void QASocketService::appConnectBootstrap(QTcpSocket *socket)
{
    socketReply(socket, 8888);
}

void QASocketService::getDeviceTimeBootstrap(QTcpSocket *socket, const QVariant &format)
{
    socketReply(socket, QDateTime::currentDateTime().toString());
}

void QASocketService::findElementBootstrap(QTcpSocket *socket, const QVariant &strategyArg, const QVariant &selectorArg)
{
    const QString strategy = strategyArg.toString().replace(QString(" "), QString(""));
    const QString selector = selectorArg.toString();

    qDebug() << Q_FUNC_INFO << socket << strategy << selector;

    QMetaObject::invokeMethod(this,
                              QStringLiteral("findStrategy_%1").arg(strategy).toLatin1().constData(),
                              Qt::QueuedConnection,
                              Q_ARG(QTcpSocket*, socket),
                              Q_ARG(QString, selector));
}

void QASocketService::findElementsBootstrap(QTcpSocket *socket, const QVariant &strategyArg, const QVariant &selectorArg, bool multiple)
{
    const QString strategy = strategyArg.toString().replace(QString(" "), QString(""));
    const QString selector = selectorArg.toString();

    qDebug() << Q_FUNC_INFO << socket << strategy << selector << multiple;

    QMetaObject::invokeMethod(this,
                              QStringLiteral("findStrategy_%1").arg(strategy).toLatin1().constData(),
                              Qt::QueuedConnection,
                              Q_ARG(QTcpSocket*, socket),
                              Q_ARG(QString, selector),
                              Q_ARG(bool, multiple));
}

void QASocketService::getLocationBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    QJsonObject reply;
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << m_items.value(elementId);
    if (m_items.contains(elementId)) {
        QQuickItem *item = m_items.value(elementId);
        const QPointF absPoint = QAEngine::getAbsPosition(item);
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

void QASocketService::getLocationInViewBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    QJsonObject reply;
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << m_items.value(elementId);
    if (m_items.contains(elementId)) {
        QQuickItem *item = m_items.value(elementId);
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

void QASocketService::getAttributeBootstrap(QTcpSocket *socket, const QVariant &attributeArg, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    const QString attribute = attributeArg.toString();
    qDebug() << Q_FUNC_INFO << elementId << attribute;
    if (m_items.contains(elementId)) {
        const QVariant reply = m_items.value(elementId)->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void QASocketService::getPropertyBootstrap(QTcpSocket *socket, const QVariant &attributeArg, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    const QString attribute = attributeArg.toString();
    qDebug() << Q_FUNC_INFO << elementId << attribute;
    if (m_items.contains(elementId)) {
        const QVariant reply = m_items.value(elementId)->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void QASocketService::getTextBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << elementId;
    if (m_items.contains(elementId)) {
        socketReply(socket, QAEngine::getText(m_items.value(elementId)));
    } else {
        socketReply(socket, QString());
    }
}

void QASocketService::getElementScreenshotBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << elementId;
    if (m_items.contains(elementId)) {
        QQuickItem *item = m_items.value(elementId);
        grabScreenshot(socket, item);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QASocketService::getScreenshotBootstrap(QTcpSocket *socket)
{
    grabScreenshot(socket, QAEngine::instance()->rootItem(), true);
}

void QASocketService::elementEnabledBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    getAttributeBootstrap(socket, "enabled", elementIdArg);
}

void QASocketService::elementDisplayedBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    getAttributeBootstrap(socket, "visible", elementIdArg);
}

void QASocketService::elementSelectedBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    getAttributeBootstrap(socket, "checked", elementIdArg);
}

void QASocketService::getSizeBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    QJsonObject reply;
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << m_items.value(elementId);
    if (m_items.contains(elementId)) {
        QQuickItem *item = m_items.value(elementId);
        reply.insert(QStringLiteral("width"), item->width());
        reply.insert(QStringLiteral("height"), item->height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void QASocketService::getNameBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    socketReply(socket, QString());
}

void QASocketService::setValueImmediateBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg)
{
    qDebug() << Q_FUNC_INFO << valueArg << elementIdArg;
    setAttribute(socket, "text", valueArg.toList().first(), elementIdArg);
}

void QASocketService::setAttribute(QTcpSocket *socket, const QVariant &attributeArg, const QVariant &valueArg, const QVariant &elementIdArg)
{
    const QString attribute = attributeArg.toString();
    const QString value = valueArg.toString();
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << attribute << value << elementId;
    if (m_items.contains(elementId)) {
        m_items.value(elementId)->setProperty(attribute.toLatin1().constData(), value);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QASocketService::replaceValueBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg)
{
    qDebug() << Q_FUNC_INFO << valueArg << elementIdArg;
    setAttribute(socket, "text", valueArg.toList().first(), elementIdArg);
}

void QASocketService::setValueBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg)
{
    qDebug() << Q_FUNC_INFO << valueArg << elementIdArg;
    QAEngine::instance()->m_keyEngine->pressKeys(valueArg.toStringList().join(""));
    socketReply(socket, QString());
}

void QASocketService::submitBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    qDebug() << Q_FUNC_INFO << elementIdArg;
    QAEngine::instance()->m_keyEngine->pressEnter(1);
    socketReply(socket, QString());
}

void QASocketService::backBootstrap(QTcpSocket *socket)
{
    m_sailfishTest->goBack();
    socketReply(socket, QString());
}

void QASocketService::forwardBootstrap(QTcpSocket *socket)
{
    m_sailfishTest->goForward();
    socketReply(socket, QString());
}

void QASocketService::activeBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QASocketService::getAlertTextBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QASocketService::isKeyboardShownBootstrap(QTcpSocket *socket)
{
    QQuickWindowPrivate *wp = QQuickWindowPrivate::get(QAEngine::instance()->rootItem()->window());
    if (!wp) {
        socketReply(socket, false, 1);
    }
    socketReply(socket, QString());
}

void QASocketService::isIMEActivatedBootstrap(QTcpSocket *socket)
{
    socketReply(socket, true);
}

void QASocketService::getOrientationBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QASocketService::setOrientationBootstrap(QTcpSocket *socket, const QVariant &orientationArg)
{
    socketReply(socket, QString());
}

void QASocketService::keyeventBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &sessionIDArg, const QVariant &flagsArg)
{
    socketReply(socket, QString());
}

void QASocketService::longPressKeyCodeBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg, const QVariant &sessionIDArg, const QVariant &paramArg)
{
    socketReply(socket, QString());
}

void QASocketService::pressKeyCodeBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg, const QVariant &sessionIDArg, const QVariant &paramArg)
{
    socketReply(socket, QString());
}

void QASocketService::hideKeyboardBootstrap(QTcpSocket *socket, const QVariant &strategyArg, const QVariant &keyArg, const QVariant &keyCodeArg, const QVariant &keyNameArg)
{
    m_sailfishTest->clearFocus();
    socketReply(socket, QString());
}

void QASocketService::executeBootstrap(QTcpSocket *socket, const QVariant &mobileArg, const QVariant &paramsArg)
{
    socketReply(socket, QString());
}

void QASocketService::performTouchBootstrap(QTcpSocket *socket, const QVariant &paramsArg)
{
    socketReply(socket, QString());
}

void QASocketService::performMultiActionBootstrap(QTcpSocket *socket, const QVariant &paramsArg, const QVariant &elementIdArg, const QVariant &sessionIdArg, const QVariant &Arg)
{
    socketReply(socket, QString());
}

void QASocketService::clickBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << elementId;
    if (m_items.contains(elementId)) {
        QQuickItem *item = m_items.value(elementId);
        m_sailfishTest->clickItem(item);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QASocketService::clearBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    setAttribute(socket, "text", "", elementIdArg);
}

void QASocketService::findStrategy_id(QTcpSocket *socket, const QString &selector, bool multiple)
{
    QQuickItem *item = QAEngine::findItemByObjectName(selector);
    qDebug() << Q_FUNC_INFO << selector << multiple << item;
    elementReply(socket, QVariantList() << QVariant::fromValue(item), multiple);
}

void QASocketService::findStrategy_classname(QTcpSocket *socket, const QString &selector, bool multiple)
{
    QVariantList items = QAEngine::findItemsByClassName(selector);
    qDebug() << Q_FUNC_INFO << selector << multiple << items;
    elementReply(socket, items, multiple);
}
void QASocketService::findStrategy_name(QTcpSocket *socket, const QString &selector, bool multiple)
{
    QVariantList items = QAEngine::findItemsByText(selector);
    qDebug() << Q_FUNC_INFO << selector << multiple << items;
    elementReply(socket, items, multiple);
}
