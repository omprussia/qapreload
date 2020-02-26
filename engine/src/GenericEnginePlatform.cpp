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

GenericEnginePlatform::GenericEnginePlatform(QObject *parent)
    : IEnginePlatform(parent)
    , m_mouseEngine(new QAMouseEngine(this))
    , m_keyEngine(new QAKeyEngine(this))
{
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
        << "Reply is:";
    qDebug().noquote()
        << data;

    socket->write(data);
    socket->flush();
}

void GenericEnginePlatform::elementReply(QTcpSocket *socket, const QVariantList &elements, bool multiple)
{
    QVariantList value;
    for (const QVariant &vitem : elements) {
        QObject *item = vitem.value<QObject*>();
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

QString GenericEnginePlatform::className(QObject *item)
{
    return QString::fromLatin1(item->metaObject()->className()).section(QChar(u'_'), 0, 0).section(QChar(u':'), -1);
}

QString GenericEnginePlatform::uniqueId(QObject *item)
{
    return QStringLiteral("%1_0x%2").arg(className(item))
            .arg(reinterpret_cast<quintptr>(item),
                 QT_POINTER_SIZE * 2, 16, QLatin1Char('0'));
}

QObject *GenericEnginePlatform::getItem(const QString &elementId)
{
    return m_items.value(elementId);
}

void GenericEnginePlatform::clickPoint(int posx, int posy)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(50);
    connect(m_mouseEngine->click(QPointF(posx, posy)),
            &QAPendingEvent::completed, &timer, static_cast<void (QTimer::*)()>(&QTimer::start));
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();
}

void GenericEnginePlatform::execute(QTcpSocket *socket, const QString &methodName, const QVariantList &params)
{
    bool handled = false;
    bool success = QAEngine::metaInvoke(socket, this, methodName, params, &handled);

    if (!handled || !success) {
        qWarning()
            << Q_FUNC_INFO
            << methodName << "not handled!";
    }
    socketReply(socket, QString());
}

void GenericEnginePlatform::activateAppCommand(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::closeAppCommand(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::queryAppStateCommand(QTcpSocket *socket, const QString &appName)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << appName;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::backgroundCommand(QTcpSocket *socket, double seconds)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << seconds;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
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
    qDebug()
        << Q_FUNC_INFO
        << socket << strategy << selector;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::findElementsCommand(QTcpSocket *socket, const QString &strategy, const QString &selector)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << strategy << selector;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::findElementFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << strategy << selector << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::findElementsFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << strategy << selector << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getLocationCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getLocationInViewCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getAttributeCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << attribute << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getPropertyCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << attribute << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getTextCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getElementScreenshotCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getScreenshotCommand(QTcpSocket *socket)
{
    qDebug()
        << Q_FUNC_INFO
        << socket;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::elementEnabledCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::elementDisplayedCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::elementSelectedCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::getSizeCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::setValueImmediateCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << value << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::replaceValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << value << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::setValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << value << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::clickCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::clearCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
}

void GenericEnginePlatform::submitCommand(QTcpSocket *socket, const QString &elementId)
{
    qDebug()
        << Q_FUNC_INFO
        << socket << elementId;
    socketReply(socket, QStringLiteral("not_implemented"), 405);
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
