#include "QAEngine.hpp"
#include "QAKeyEngine.hpp"
#include "SailfishTest.hpp"
#include "qaservice_adaptor.h"
#include "QASocketService.hpp"

#include <QTcpServer>
#include <QTcpSocket>

#include <QPainter>
#include <QPixmap>
#include <QQuickItem>
#include <QQuickItemGrabResult>

#include <QGuiApplication>
#include <QClipboard>
#include <QQmlExpression>
#include <QQmlEngine>

#include <private/qquickwindow_p.h>

#include <mlite5/MGConfItem>

static QASocketService *s_instance = nullptr;
static QHash<QString, QQuickItem*> s_items;

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
        s_items.insert(uId, item);

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
    if (!socket) {
        return;
    }

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
    QVariantList params = object.value(QStringLiteral("params")).toVariant().toList();
    if (params.length() > 9) {
        qWarning() << Q_FUNC_INFO << "Too many params for" << action;
        params = params.mid(0, 9);
    }

    QGenericArgument arguments[9] = { QGenericArgument() };
    for (int i = 0; i < params.length(); i++) {
        arguments[i] = Q_ARG(QVariant, params[i]);
    }

    QMetaObject::invokeMethod(this,
                              QStringLiteral("%1Bootstrap").arg(action).toLatin1().constData(),
                              Qt::DirectConnection,
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

void QASocketService::activateAppBootstrap(QTcpSocket *socket, const QVariant &appIdArg)
{
    const QString appName = appIdArg.toString();
    if (appName != qApp->arguments().first().section(QLatin1Char('/'), -1)) {
        qWarning() << Q_FUNC_INFO << appName << "is not" << qApp->arguments().first().section(QLatin1Char('/'), -1);
        socketReply(socket, QString(), 1);
        return;
    }
    QAEngine::instance()->rootItem()->window()->raise();
    socketReply(socket, QString());
}

void QASocketService::closeAppBootstrap(QTcpSocket *socket, const QVariant &appIdArg)
{
    const QString appName = appIdArg.toString();
    qDebug() << Q_FUNC_INFO << appName;
    if (appName != qApp->arguments().first().section(QLatin1Char('/'), -1)) {
        qWarning() << Q_FUNC_INFO << appName << "is not" << qApp->arguments().first().section(QLatin1Char('/'), -1);
        socketReply(socket, false, 1);
        return;
    }
    socketReply(socket, true);
    qApp->quit();
}

void QASocketService::queryAppStateBootstrap(QTcpSocket *socket, const QVariant &appIdArg)
{
    const QString appName = appIdArg.toString();
    qDebug() << Q_FUNC_INFO << appName;
    if (appName != qApp->arguments().first().section(QLatin1Char('/'), -1)) {
        qWarning() << Q_FUNC_INFO << appName << "is not" << qApp->arguments().first().section(QLatin1Char('/'), -1);
        socketReply(socket, QString(), 1);
        return;
    }
    const bool isAppActive = QAEngine::instance()->rootItem()->window()->isActive();
    socketReply(socket, isAppActive ? QStringLiteral("RUNNING_IN_FOREGROUND") : QStringLiteral("RUNNING_IN_BACKGROUND"));
}

void QASocketService::getClipboardBootstrap(QTcpSocket *socket, const QVariant &)
{
    socketReply(socket, QString::fromUtf8(qGuiApp->clipboard()->text().toUtf8().toBase64()));
}

void QASocketService::setClipboardBootstrap(QTcpSocket *socket, const QVariant &contentArg, const QVariant &)
{
    qGuiApp->clipboard()->setText(contentArg.toString());
    socketReply(socket, QString());
}

void QASocketService::implicitWaitBootstrap(QTcpSocket *socket, const QVariant &msecondArg)
{
    int msecs = msecondArg.toInt();

    socketReply(socket, QString());
}

void QASocketService::backgroundBootstrap(QTcpSocket *socket, const QVariant &secondsArg)
{
    int msecs = secondsArg.toInt() * 1000;
    qDebug() << Q_FUNC_INFO << msecs;
    QAEngine::instance()->rootItem()->window()->lower();
    if (msecs > 0) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(msecs);
        loop.exec();
        QAEngine::instance()->rootItem()->window()->raise();
    }
    socketReply(socket, QString());
}

void QASocketService::findElementBootstrap(QTcpSocket *socket, const QVariant &strategyArg, const QVariant &selectorArg)
{
    const QString strategy = strategyArg.toString().replace(QChar(' '), QString());
    const QString selector = selectorArg.toString();

    qDebug() << Q_FUNC_INFO << socket << strategy << selector;

    QMetaObject::invokeMethod(this,
                              QStringLiteral("findStrategy_%1").arg(strategy).toLatin1().constData(),
                              Qt::DirectConnection,
                              Q_ARG(QTcpSocket*, socket),
                              Q_ARG(QString, selector));
}

void QASocketService::findElementsBootstrap(QTcpSocket *socket, const QVariant &strategyArg, const QVariant &selectorArg, bool multiple)
{
    const QString strategy = strategyArg.toString().replace(QChar(' '), QString());
    const QString selector = selectorArg.toString();

    qDebug() << Q_FUNC_INFO << socket << strategy << selector << multiple;

    QMetaObject::invokeMethod(this,
                              QStringLiteral("findStrategy_%1").arg(strategy).toLatin1().constData(),
                              Qt::DirectConnection,
                              Q_ARG(QTcpSocket*, socket),
                              Q_ARG(QString, selector),
                              Q_ARG(bool, multiple));
}

void QASocketService::getLocationBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    QJsonObject reply;
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << s_items.value(elementId);
    if (s_items.contains(elementId)) {
        QQuickItem *item = s_items.value(elementId);
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
    qDebug() << Q_FUNC_INFO << s_items.value(elementId);
    if (s_items.contains(elementId)) {
        QQuickItem *item = s_items.value(elementId);
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
    if (s_items.contains(elementId)) {
        const QVariant reply = s_items.value(elementId)->property(attribute.toLatin1().constData());
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
    if (s_items.contains(elementId)) {
        const QVariant reply = s_items.value(elementId)->property(attribute.toLatin1().constData());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void QASocketService::getTextBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << elementId;
    if (s_items.contains(elementId)) {
        socketReply(socket, QAEngine::getText(s_items.value(elementId)));
    } else {
        socketReply(socket, QString());
    }
}

void QASocketService::getElementScreenshotBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << elementId;
    if (s_items.contains(elementId)) {
        QQuickItem *item = s_items.value(elementId);
        grabScreenshot(socket, item);
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QASocketService::getScreenshotBootstrap(QTcpSocket *socket)
{
    grabScreenshot(socket, QAEngine::getApplicationWindow(), true);
}

void QASocketService::elementEnabledBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    getAttributeBootstrap(socket, QStringLiteral("enabled"), elementIdArg);
}

void QASocketService::elementDisplayedBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    getAttributeBootstrap(socket, QStringLiteral("visible"), elementIdArg);
}

void QASocketService::elementSelectedBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    getAttributeBootstrap(socket, QStringLiteral("checked"), elementIdArg);
}

void QASocketService::getSizeBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << s_items.value(elementId);
    if (s_items.contains(elementId)) {
        QQuickItem *item = s_items.value(elementId);
        QJsonObject reply;
        reply.insert(QStringLiteral("width"), item->width());
        reply.insert(QStringLiteral("height"), item->height());
        socketReply(socket, reply);
    } else {
        socketReply(socket, QString());
    }
}

void QASocketService::setValueImmediateBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg)
{
    qDebug() << Q_FUNC_INFO << valueArg << elementIdArg;
    setAttribute(socket, QStringLiteral("text"), valueArg.toList().first(), elementIdArg);
}

void QASocketService::setAttribute(QTcpSocket *socket, const QVariant &attributeArg, const QVariant &valueArg, const QVariant &elementIdArg)
{
    const QString attribute = attributeArg.toString();
    const QString value = valueArg.toString();
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << attribute << value << elementId;
    if (s_items.contains(elementId)) {
        s_items.value(elementId)->setProperty(attribute.toLatin1().constData(), value);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QASocketService::replaceValueBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg)
{
    qDebug() << Q_FUNC_INFO << valueArg << elementIdArg;
    setAttribute(socket, QStringLiteral("text"), valueArg.toList().first(), elementIdArg);
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

void QASocketService::getCurrentActivityBootstrap(QTcpSocket *socket)
{
    QQuickItem* currentPage = QAEngine::instance()->getCurrentPage();
    const QString currentPageName = QStringLiteral("%1_%2")
            .arg(QString::fromLatin1(currentPage->metaObject()->className()).section(QChar('_'), 0, 0))
            .arg(currentPage->property("objectName").toString());
    socketReply(socket, currentPageName);
}

void QASocketService::getPageSourceBootstrap(QTcpSocket *socket)
{
    QQuickItem *currentPage = QAEngine::instance()->getCurrentPage();
    QJsonObject reply = QAEngine::instance()->recursiveDumpTree(currentPage);
    socketReply(socket, QJsonDocument(reply).toJson(QJsonDocument::Compact));
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
    QInputMethod* ime = qApp->inputMethod();
    if (!ime) {
        socketReply(socket, false, 1);
    }
    socketReply(socket, ime->isVisible());
}

void QASocketService::availableIMEEnginesBootstrap(QTcpSocket *socket)
{
    MGConfItem enabled(QStringLiteral("/sailfish/text_input/enabled_layouts"));
    socketReply(socket, enabled.value().toStringList());
}

void QASocketService::activateIMEEngineBootstrap(QTcpSocket *socket, const QVariant &engine)
{
    const QString layout = engine.toString();
    qDebug() << Q_FUNC_INFO << layout;
    MGConfItem enabled(QStringLiteral("/sailfish/text_input/enabled_layouts"));
    if (!enabled.value().toStringList().contains(layout)) {
        socketReply(socket, QString());
        return;
    }

    MGConfItem active(QStringLiteral("/sailfish/text_input/active_layout"));
    active.set(layout);

    socketReply(socket, QString());
}

void QASocketService::getActiveIMEEngineBootstrap(QTcpSocket *socket)
{
    MGConfItem active(QStringLiteral("/sailfish/text_input/active_layout"));
    socketReply(socket, active.value().toString());
}

void QASocketService::deactivateIMEEngineBootstrap(QTcpSocket *socket)
{
    socketReply(socket, QString());
}

void QASocketService::isIMEActivatedBootstrap(QTcpSocket *socket)
{
    socketReply(socket, qApp->inputMethod() ? true : false);
}


void QASocketService::getOrientationBootstrap(QTcpSocket *socket)
{
    QQuickItem* item = QAEngine::getApplicationWindow();
    const int deviceOrientation = item->property("deviceOrientation").toInt();
    socketReply(socket, deviceOrientation == 2 || deviceOrientation == 8 ? QStringLiteral("LANDSCAPE") : QStringLiteral("PORTRAIT"));
}

void QASocketService::setOrientationBootstrap(QTcpSocket *socket, const QVariant &orientationArg)
{
    QQuickItem* item = QAEngine::getApplicationWindow();
    const QString orientation = orientationArg.toString();
    item->setProperty("deviceOrientation", orientation == QStringLiteral("LANDSCAPE") ? 2 : 1);
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

void QASocketService::executeBootstrap(QTcpSocket *socket, const QVariant &commandArg, const QVariant &paramsArg)
{
    const QString command = commandArg.toString().replace(QChar(':'), QChar('_'));

    qDebug() << Q_FUNC_INFO << socket << command << paramsArg;

    const QVariantList params = paramsArg.toList();
    QGenericArgument arguments[9] = { QGenericArgument() };
    for (int i = 0; i < params.length(); i++) {
        arguments[i] = Q_ARG(QVariant, params[i]);
    }

    bool handled = QMetaObject::invokeMethod(this,
                              QStringLiteral("executeCommand_%1").arg(command).toLatin1().constData(),
                              Qt::DirectConnection,
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

    if (!handled) {
        qWarning() << Q_FUNC_INFO << command << "not handled!";
        socketReply(socket, QString());
    }
}

void QASocketService::executeAsyncBootstrap(QTcpSocket *socket, const QVariant &commandArg, const QVariant &paramsArg)
{
    const QString command = commandArg.toString().replace(QChar(':'), QChar('_'));

    qDebug() << Q_FUNC_INFO << socket << command << paramsArg;

    const QVariantList params = paramsArg.toList();
    QGenericArgument arguments[9] = { QGenericArgument() };
    for (int i = 0; i < params.length(); i++) {
        arguments[i] = Q_ARG(QVariant, params[i]);
    }

    bool handled = QMetaObject::invokeMethod(this,
                              QStringLiteral("executeCommand_%1").arg(command).toLatin1().constData(),
                              Qt::QueuedConnection,
                              Q_ARG(QTcpSocket*, nullptr),
                              arguments[0],
                              arguments[1],
                              arguments[2],
                              arguments[3],
                              arguments[4],
                              arguments[5],
                              arguments[6],
                              arguments[7],
                              arguments[8]);

    if (!handled) {
        qWarning() << Q_FUNC_INFO << command << "not handled!";
    }
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_pullDownTo(QTcpSocket *socket, const QVariant &destinationArg)
{
    qDebug() << Q_FUNC_INFO << destinationArg;
    if (destinationArg.type() == QMetaType::QString) {
        const QString name = destinationArg.toString();
        m_sailfishTest->pullDownTo(name);
    } else {
        const int index = destinationArg.toInt();
        m_sailfishTest->pullDownTo(index);
    }
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_pushUpTo(QTcpSocket *socket, const QVariant &destinationArg)
{
    qDebug() << Q_FUNC_INFO << destinationArg;
    if (destinationArg.type() == QMetaType::QString) {
        const QString name = destinationArg.toString();
        m_sailfishTest->pushUpTo(name);
    } else {
        const int index = destinationArg.toInt();
        m_sailfishTest->pushUpTo(index);
    }
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_clickContextMenuItem(QTcpSocket *socket, const QVariant& elementIdArg, const QVariant &destinationArg)
{
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << elementId << destinationArg;
    if (s_items.contains(elementId)) {
        if (destinationArg.type() == QMetaType::QString) {
            const QString name = destinationArg.toString();
            m_sailfishTest->clickContextMenuItem(s_items.value(elementId), name);
        } else {
            const int index = destinationArg.toInt();
            m_sailfishTest->clickContextMenuItem(s_items.value(elementId), index);
        }
    }
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_waitForPageChange(QTcpSocket *socket)
{
    m_sailfishTest->waitForPageChange();
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_swipe(QTcpSocket *socket, const QVariant &directionArg)
{
    const QString directionString = directionArg.toString();
    SailfishTest::SwipeDirection direction = SailfishTest::SwipeDirectionDown;
    if (directionString == QStringLiteral("down")) {
        direction = SailfishTest::SwipeDirectionDown;
    } else if (directionString == QStringLiteral("up")) {
        direction = SailfishTest::SwipeDirectionUp;
    } else if (directionString == QStringLiteral("left")) {
        direction = SailfishTest::SwipeDirectionLeft;
    } else if (directionString == QStringLiteral("right")) {
        direction = SailfishTest::SwipeDirectionRight;
    }
    m_sailfishTest->swipe(direction);
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_peek(QTcpSocket *socket, const QVariant &directionArg)
{
    const QString directionString = directionArg.toString();
    SailfishTest::PeekDirection direction = SailfishTest::PeekDirectionDown;
    if (directionString == QStringLiteral("down")) {
        direction = SailfishTest::PeekDirectionDown;
    } else if (directionString == QStringLiteral("up")) {
        direction = SailfishTest::PeekDirectionUp;
    } else if (directionString == QStringLiteral("left")) {
        direction = SailfishTest::PeekDirectionLeft;
    } else if (directionString == QStringLiteral("right")) {
        direction = SailfishTest::PeekDirectionRight;
    }
    m_sailfishTest->peek(direction);
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_goBack(QTcpSocket *socket)
{
    m_sailfishTest->goBack();
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_goForward(QTcpSocket *socket)
{
    m_sailfishTest->goForward();
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_enterCode(QTcpSocket *socket, const QVariant &codeArg)
{
    m_sailfishTest->enterCode(codeArg.toString());
    socketReply(socket, QString());
}

void QASocketService::executeCommand_touch_pressAndHold(QTcpSocket *socket, const QVariant &posxArg, const QVariant &posyArg)
{
    m_sailfishTest->pressAndHold(posxArg.toInt(), posyArg.toInt());
    socketReply(socket, QString());
}

void QASocketService::executeCommand_touch_mouseSwipe(QTcpSocket *socket, const QVariant &posxArg, const QVariant &posyArg, const QVariant &stopxArg, const QVariant &stopyArg)
{
    m_sailfishTest->mouseSwipe(posxArg.toInt(), posyArg.toInt(), stopxArg.toInt(), stopyArg.toInt());
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_scrollToItem(QTcpSocket *socket, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << elementId;
    if (s_items.contains(elementId)) {
        m_sailfishTest->scrollToItem(s_items.value(elementId));
    }
    socketReply(socket, QString());
}

void QASocketService::executeCommand_app_method(QTcpSocket *socket, const QVariant &elementIdArg, const QVariant &methodArg, const QVariant& paramsArg)
{
    const QString elementId = elementIdArg.toString();
    QObject* object = QAEngine::getApplicationWindow()->window();
    if (s_items.contains(elementId)) {
        object = s_items.value(elementId);
    }

    const QVariantList params = paramsArg.toList();
    QGenericArgument arguments[10] = { QGenericArgument() };
    for (int i = 0; i < params.length(); i++) {
        arguments[i] = Q_ARG(QVariant, params[i]);
    }

    QVariant reply;

    QMetaObject::invokeMethod(object,
                              methodArg.toString().toLatin1().constData(),
                              Qt::DirectConnection,
                              Q_RETURN_ARG(QVariant, reply),
                              arguments[0],
                              arguments[1],
                              arguments[2],
                              arguments[3],
                              arguments[4],
                              arguments[5],
                              arguments[6],
                              arguments[7],
                              arguments[8],
                              arguments[9]);
    socketReply(socket, reply);
}

void QASocketService::executeCommand_app_js(QTcpSocket *socket, const QVariant &elementIdArg, const QVariant &jsCodeArg)
{
    const QString elementId = elementIdArg.toString();
    const QString jsCode = jsCodeArg.toString();

    QQuickItem* item = QAEngine::getApplicationWindow();
    QObject* object = item->window();
    if (s_items.contains(elementId)) {
        item = s_items.value(elementId);
    }

    if (!qmlEngine(item)) {
        qWarning() << Q_FUNC_INFO << "No Engine for" << item;
        socketReply(socket, QString());
        return;
    }

    QQmlExpression expr(qmlEngine(QAEngine::getApplicationWindow())->rootContext(), object, jsCode);
    bool isUndefined = false;
    const QVariant reply = expr.evaluate(&isUndefined);
    if (expr.hasError()) {
        qWarning() << Q_FUNC_INFO << expr.error().toString();
    }

    qDebug() << Q_FUNC_INFO << reply;
    socketReply(socket, QString());
}

void QASocketService::performTouchBootstrap(QTcpSocket *socket, const QVariant &paramsArg)
{    
    processTouchActionList(paramsArg);

    socketReply(socket, QString());
}

void QASocketService::performMultiActionBootstrap(QTcpSocket *socket, const QVariant &paramsArg, const QVariant &elementIdArg, const QVariant &, const QVariant &)
{
    for (const QVariant &actionListArg : paramsArg.toList()) {
        processTouchActionList(actionListArg);
    }

    socketReply(socket, QString());
}

void QASocketService::processTouchActionList(const QVariant &actionListArg)
{
    int startX = 0;
    int startY = 0;
    int endX = 0;
    int endY = 0;
    int delay = 0;

    const QVariantList actions = actionListArg.toList();
    for (const QVariant &actionArg : actions) {
        const QVariantMap action = actionArg.toMap();
        const QString actionName = action.value(QStringLiteral("action")).toString();
        const QVariantMap options = action.value(QStringLiteral("options")).toMap();

        if (actionName == QStringLiteral("wait")) {
            delay = options.value(QStringLiteral("ms")).toInt();
        } else if (actionName == QStringLiteral("press")) {
            startX = options.value(QStringLiteral("x")).toInt();
            startY = options.value(QStringLiteral("y")).toInt();
        } else if (actionName == QStringLiteral("moveTo")) {
            endX = options.value(QStringLiteral("x")).toInt();
            endY = options.value(QStringLiteral("y")).toInt();
        } else if (actionName == QStringLiteral("release")) {
            m_sailfishTest->mouseDrag(startX, startY, endX, endY, delay);
        } else if (actionName == QStringLiteral("longPress")) {
            const QString elementId = options.value(QStringLiteral("element")).toString();
            if (!s_items.contains(elementId)) {
                continue;
            }
            m_sailfishTest->pressAndHold(s_items.value(elementId));
        }
    }
}

void QASocketService::startActivityBootstrap(QTcpSocket *socket, const QVariant &appPackage, const QVariant &appActivity, const QVariant &appWaitPackage, const QVariant &intentAction, const QVariant &intentCategory, const QVariant &intentFlags, const QVariant &optionalIntentArguments, const QVariant &dontStopAppOnReset, const QVariant &)
{
    socketReply(socket, QString());
}

void QASocketService::clickBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    const QString elementId = elementIdArg.toString();
    qDebug() << Q_FUNC_INFO << elementId;
    if (s_items.contains(elementId)) {
        QQuickItem *item = s_items.value(elementId);
        m_sailfishTest->clickItem(item);
        socketReply(socket, QString());
    } else {
        socketReply(socket, QString(), 1);
    }
}

void QASocketService::clearBootstrap(QTcpSocket *socket, const QVariant &elementIdArg)
{
    setAttribute(socket, QStringLiteral("text"), QString(), elementIdArg);
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
