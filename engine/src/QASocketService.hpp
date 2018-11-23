#ifndef QASOCKETSERVICE_HPP
#define QASOCKETSERVICE_HPP

#include <QObject>

class QTcpServer;
class QTcpSocket;
class QQuickItem;
class SailfishTest;
class QASocketService: public QObject
{
    Q_OBJECT
public:
    static QASocketService *instance();
    void setAttribute(QTcpSocket *socket, const QString &attribute, const QString &value, const QString &elementId);

public slots:
    quint16 serverPort();
    bool isListening();
    void stopListen();

private slots:
    void newConnection();
    void readSocket();

    void activateAppBootstrap(QTcpSocket *socket, const QString &appName);
    void closeAppBootstrap(QTcpSocket *socket, const QString &appName);
    void queryAppStateBootstrap(QTcpSocket *socket, const QString &appName);
    void backgroundBootstrap(QTcpSocket *socket, double seconds);
    void getClipboardBootstrap(QTcpSocket *socket);
    void setClipboardBootstrap(QTcpSocket *socket, const QString &content);
    void findElementBootstrap(QTcpSocket *socket, const QString &strategy, const QString &selector);
    void findElementsBootstrap(QTcpSocket *socket, const QString &strategy, const QString &selector);
    void findElementFromElementBootstrap(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId);
    void findElementsFromElementBootstrap(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId);
    void getLocationBootstrap(QTcpSocket *socket, const QString &elementId);
    void getLocationInViewBootstrap(QTcpSocket *socket, const QString &elementId);
    void getAttributeBootstrap(QTcpSocket *socket, const QString &attribute, const QString &elementId);
    void getPropertyBootstrap(QTcpSocket *socket, const QString &attribute, const QString &elementId);
    void getTextBootstrap(QTcpSocket *socket, const QString &elementId);
    void getElementScreenshotBootstrap(QTcpSocket *socket, const QString &elementId);
    void getScreenshotBootstrap(QTcpSocket *socket);
    void elementEnabledBootstrap(QTcpSocket *socket, const QString &elementId);
    void elementDisplayedBootstrap(QTcpSocket *socket, const QString &elementId);
    void elementSelectedBootstrap(QTcpSocket *socket, const QString &elementId);
    void getSizeBootstrap(QTcpSocket *socket, const QString &elementId);
//    void getNameBootstrap(QTcpSocket *socket, const QVariant &elementIdArg); not supported tag_name
    void setValueImmediateBootstrap(QTcpSocket *socket, const QVariantList &value, const QString &elementId);
    void replaceValueBootstrap(QTcpSocket *socket, const QVariantList &value, const QString &elementId);
    void setValueBootstrap(QTcpSocket *socket, const QVariantList &value, const QString &elementId);
    void clickBootstrap(QTcpSocket *socket, const QString &elementId);
    void clearBootstrap(QTcpSocket *socket, const QString &elementId);
    void submitBootstrap(QTcpSocket *socket, const QString &elementId);
    void getPageSourceBootstrap(QTcpSocket *socket);
    void backBootstrap(QTcpSocket *socket);
    void forwardBootstrap(QTcpSocket *socket);
    void getOrientationBootstrap(QTcpSocket *socket);
    void setOrientationBootstrap(QTcpSocket *socket, const QString &orientation);
    void hideKeyboardBootstrap(QTcpSocket *socket, const QString &strategy, const QString &key = QString(), double keyCode = 0, const QString &keyName = QString());

    void getCurrentActivityBootstrap(QTcpSocket *socket);
    void implicitWaitBootstrap(QTcpSocket *socket, double msecs); // ?
    void activeBootstrap(QTcpSocket *socket); //for functions switch_to.active_element
    void getAlertTextBootstrap(QTcpSocket *socket); //for functions switch_to.alert
    void isKeyboardShownBootstrap(QTcpSocket *socket);
    void activateIMEEngineBootstrap(QTcpSocket *socket, const QVariant &engine);
    void availableIMEEnginesBootstrap(QTcpSocket *socket);
    void getActiveIMEEngineBootstrap(QTcpSocket *socket);
    void deactivateIMEEngineBootstrap(QTcpSocket *socket);
    void isIMEActivatedBootstrap(QTcpSocket *socket);
    void keyeventBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &sessionIDArg, const QVariant &flagsArg);
    void longPressKeyCodeBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg);
    void pressKeyCodeBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg);

    void executeBootstrap(QTcpSocket *socket, const QString &command, const QVariantList &params);
    void executeAsyncBootstrap(QTcpSocket *socket, const QString &command, const QVariantList &params);

    void executeCommand_app_pullDownTo(QTcpSocket *socket, const QString &destination);
    void executeCommand_app_pullDownTo(QTcpSocket *socket, double destination);
    void executeCommand_app_pushUpTo(QTcpSocket *socket, const QString &destination);
    void executeCommand_app_pushUpTo(QTcpSocket *socket, double destination);

    void executeCommand_app_clickContextMenuItem(QTcpSocket *socket, const QString &elementId, const QString &destination);
    void executeCommand_app_clickContextMenuItem(QTcpSocket *socket, const QString &elementId, double destination);

    void executeCommand_app_waitForPageChange(QTcpSocket *socket, double timeout = 3000);
    void executeCommand_app_waitForPropertyChange(QTcpSocket *socket, const QString &elementId, const QString &propertyName, const QVariant &value, double timeout = 3000);

    void executeCommand_app_swipe(QTcpSocket *socket, const QString &directionString);
    void executeCommand_app_peek(QTcpSocket *socket, const QString &directionString);

    void executeCommand_app_goBack(QTcpSocket *socket);
    void executeCommand_app_goForward(QTcpSocket *socket);

    void executeCommand_app_enterCode(QTcpSocket *socket, const QString &code);

    void executeCommand_touch_pressAndHold(QTcpSocket *socket, double posx, double posy);
    void executeCommand_touch_mouseSwipe(QTcpSocket *socket, double posx, double posy, double stopx, double stopy);

    void executeCommand_app_scrollToItem(QTcpSocket *socket, const QString &elementId);

    void executeCommand_app_method(QTcpSocket *socket, const QString &elementId, const QString &method, const QVariantList &params);
    void executeCommand_app_js(QTcpSocket *socket, const QString &elementId, const QString &jsCode);

    void executeCommand_app_setAttribute(QTcpSocket *socket, const QString &elementId, const QString &attribute, const QString &value);

    void executeCommand_app_dumpCurrentPage(QTcpSocket *socket);
    void executeCommand_app_dumpTree(QTcpSocket *socket);

    void performTouchBootstrap(QTcpSocket *socket, const QVariant &paramsArg);
    void performMultiActionBootstrap(QTcpSocket *socket, const QVariant &paramsArg, const QVariant &elementIdArg);
    void performMultiActionBootstrap(QTcpSocket *socket, const QVariantList &actions);

    void processTouchActionList(const QVariant &actionListArg);

    void findByProperty(QTcpSocket *socket, const QString &propertyName, const QVariant &propertyValue, bool multiple = false, QQuickItem *parentItem = nullptr);

    void findStrategy_id(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);
    void findStrategy_objectName(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);
    void findStrategy_classname(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);
    void findStrategy_name(QTcpSocket *socket, const QString &selector, bool multiple = false, QQuickItem *parentItem = nullptr);

private:
    explicit QASocketService(QObject *parent = nullptr);
    void initialize();

    bool invoke(QTcpSocket *socket, const QString &methodName, const QVariantList &parameters);

    void elementReply(QTcpSocket *socket, const QVariantList &elements, bool multiple = false);
    void socketReply(QTcpSocket *socket, const QVariant &value, int status = 0);

    void grabScreenshot(QTcpSocket *socket, QQuickItem *item, bool fillBackground = false);

    QTcpServer *m_server = nullptr;

    SailfishTest *m_sailfishTest = nullptr;
};

#endif // QASOCKETSERVICE_HPP
