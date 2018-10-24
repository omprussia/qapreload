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
    void setAttribute(QTcpSocket *socket, const QVariant &attributeArg, const QVariant &valueArg, const QVariant &elementIdArg);

public slots:
    quint16 serverPort();
    bool isListening();
    void stopListen();

private slots:
    void newConnection();
    void readSocket();

    void activateAppBootstrap(QTcpSocket *socket, const QVariant &appIdArg);
    void closeAppBootstrap(QTcpSocket *socket, const QVariant &appIdArg);
    void queryAppStateBootstrap(QTcpSocket *socket, const QVariant &appIdArg);
    void backgroundBootstrap(QTcpSocket *socket, const QVariant &secondsArg);
    void getClipboardBootstrap(QTcpSocket *socket, const QVariant &);
    void setClipboardBootstrap(QTcpSocket *socket, const QVariant &contentArg, const QVariant &);
    void findElementBootstrap(QTcpSocket *socket, const QVariant &strategyArg, const QVariant &selectorArg);
    void findElementsBootstrap(QTcpSocket *socket, const QVariant &strategyArg, const QVariant &selectorArg, bool multiple = true);
    void getLocationBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void getLocationInViewBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void getAttributeBootstrap(QTcpSocket *socket, const QVariant &attributeArg, const QVariant &elementIdArg);
    void getPropertyBootstrap(QTcpSocket *socket, const QVariant &attributeArg, const QVariant &elementIdArg);
    void getTextBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void getElementScreenshotBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void getScreenshotBootstrap(QTcpSocket *socket);
    void elementEnabledBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void elementDisplayedBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void elementSelectedBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void getSizeBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
//    void getNameBootstrap(QTcpSocket *socket, const QVariant &elementIdArg); not supported tag_name
    void setValueImmediateBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg);
    void replaceValueBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg);
    void setValueBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg);
    void clickBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void clearBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void submitBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void getPageSourceBootstrap(QTcpSocket *socket);
    void backBootstrap(QTcpSocket *socket);
    void forwardBootstrap(QTcpSocket *socket);
    void getOrientationBootstrap(QTcpSocket *socket);
    void setOrientationBootstrap(QTcpSocket *socket, const QVariant &orientationArg);
    void hideKeyboardBootstrap(QTcpSocket *socket, const QVariant &strategyArg, const QVariant &keyArg, const QVariant &keyCodeArg, const QVariant &keyNameArg);

    void getCurrentActivityBootstrap(QTcpSocket *socket);
    void implicitWaitBootstrap(QTcpSocket *socket, const QVariant &msecondArg); // ?
    void activeBootstrap(QTcpSocket *socket); //for functions switch_to.active_element
    void getAlertTextBootstrap(QTcpSocket *socket); //for functions switch_to.alert
    void isKeyboardShownBootstrap(QTcpSocket *socket);
    void activateIMEEngineBootstrap(QTcpSocket *socket, const QVariant &engine);
    void availableIMEEnginesBootstrap(QTcpSocket *socket);
    void getActiveIMEEngineBootstrap(QTcpSocket *socket);
    void deactivateIMEEngineBootstrap(QTcpSocket *socket);
    void isIMEActivatedBootstrap(QTcpSocket *socket);
    void keyeventBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &sessionIDArg, const QVariant &flagsArg);
    void longPressKeyCodeBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg, const QVariant &sessionIDArg, const QVariant &paramArg);
    void pressKeyCodeBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg, const QVariant &sessionIDArg, const QVariant &paramArg);

    void executeBootstrap(QTcpSocket *socket, const QVariant &commandArg, const QVariant &paramsArg);
    void executeAsyncBootstrap(QTcpSocket *socket, const QVariant &commandArg, const QVariant &paramsArg);

    void executeCommand_app_pullDownTo(QTcpSocket* socket, const QVariant &destinationArg);
    void executeCommand_app_pushUpTo(QTcpSocket* socket, const QVariant &destinationArg);

    void executeCommand_app_clickContextMenuItem(QTcpSocket* socket, const QVariant &elementIdArg, const QVariant &destinationArg);

    void executeCommand_app_waitForPageChange(QTcpSocket* socket);

    void executeCommand_app_swipe(QTcpSocket* socket, const QVariant& directionArg);
    void executeCommand_app_peek(QTcpSocket* socket, const QVariant& directionArg);

    void executeCommand_app_goBack(QTcpSocket* socket);
    void executeCommand_app_goForward(QTcpSocket* socket);

    void executeCommand_app_enterCode(QTcpSocket* socket, const QVariant& codeArg);

    void executeCommand_touch_pressAndHold(QTcpSocket* socket, const QVariant& posxArg, const QVariant& posyArg);
    void executeCommand_touch_mouseSwipe(QTcpSocket* socket, const QVariant& posxArg, const QVariant& posyArg, const QVariant& stopxArg, const QVariant& stopyArg);

    void executeCommand_app_scrollToItem(QTcpSocket* socket, const QVariant& elementIdArg);

    void executeCommand_app_method(QTcpSocket *socket, const QVariant &elementIdArg, const QVariant &methodArg, const QVariant& paramsArg);
    void executeCommand_app_js(QTcpSocket* socket, const QVariant& elementIdArg, const QVariant& jsCodeArg);

    void executeCommand_app_dumpCurrentPage(QTcpSocket* socket);
    void executeCommand_app_dumpTree(QTcpSocket* socket);

    void performTouchBootstrap(QTcpSocket *socket, const QVariant &paramsArg);
    void performMultiActionBootstrap(QTcpSocket *socket, const QVariant &paramsArg, const QVariant &elementIdArg, const QVariant &, const QVariant &);

    void processTouchActionList(const QVariant &actionListArg);

    void startActivityBootstrap(QTcpSocket *socket, const QVariant &appPackage, const QVariant &appActivity, const QVariant &appWaitPackage, const QVariant &intentAction,
                                const QVariant &intentCategory, const QVariant &intentFlags, const QVariant &optionalIntentArguments, const QVariant &dontStopAppOnReset, const QVariant &); //Segmentation fauilt

    void findStrategy_id(QTcpSocket *socket, const QString &selector, bool multiple = false);
    void findStrategy_classname(QTcpSocket *socket, const QString &selector, bool multiple = false);
    void findStrategy_name(QTcpSocket *socket, const QString &selector, bool multiple = false);

private:
    explicit QASocketService(QObject *parent = nullptr);
    void initialize();

    void elementReply(QTcpSocket *socket, const QVariantList &elements, bool multiple = false);
    void socketReply(QTcpSocket *socket, const QVariant &value, int status = 0);

    void grabScreenshot(QTcpSocket *socket, QQuickItem *item, bool fillBackground = false);

    QTcpServer *m_server = nullptr;

    SailfishTest *m_sailfishTest = nullptr;
};

#endif // QASOCKETSERVICE_HPP
