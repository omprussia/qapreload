#ifndef QASOCKETSERVICE_HPP
#define QASOCKETSERVICE_HPP

#include <QObject>
#include <QQuickItem>
#include <QTcpServer>
#include <QTcpSocket>

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

    void appConnectBootstrap(QTcpSocket *socket);
    void getDeviceTimeBootstrap(QTcpSocket *socket, const QVariant &format = QVariant());
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
    void getNameBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void setValueImmediateBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg);
    void replaceValueBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg);
    void setValueBootstrap(QTcpSocket *socket, const QVariant &valueArg, const QVariant &elementIdArg);
    void clickBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void clearBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);
    void submitBootstrap(QTcpSocket *socket, const QVariant &elementIdArg);

    void backBootstrap(QTcpSocket *socket);
    void forwardBootstrap(QTcpSocket *socket);
    void activeBootstrap(QTcpSocket *socket); //for functions switch_to.active_element
    void getAlertTextBootstrap(QTcpSocket *socket); //for functions switch_to.alert
    void isKeyboardShownBootstrap(QTcpSocket *socket);
    void isIMEActivatedBootstrap(QTcpSocket *socket);
    void getOrientationBootstrap(QTcpSocket *socket);
    void setOrientationBootstrap(QTcpSocket *socket, const QVariant &orientationArg);
    void keyeventBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &sessionIDArg, const QVariant &flagsArg);
    void longPressKeyCodeBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg, const QVariant &sessionIDArg, const QVariant &paramArg);
    void pressKeyCodeBootstrap(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg, const QVariant &sessionIDArg, const QVariant &paramArg);
    void hideKeyboardBootstrap(QTcpSocket *socket, const QVariant &strategyArg, const QVariant &keyArg, const QVariant &keyCodeArg, const QVariant &keyNameArg);
    void executeBootstrap(QTcpSocket *socket, const QVariant &mobileArg, const QVariant &paramsArg);
    void performTouchBootstrap(QTcpSocket *socket, const QVariant &paramsArg);
    void performMultiActionBootstrap(QTcpSocket *socket, const QVariant &paramsArg, const QVariant &elementIdArg, const QVariant &sessionIdArg, const QVariant &Arg);

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
    QHash<QString, QQuickItem*> m_items;

    SailfishTest *m_sailfishTest = nullptr;
};

#endif // QASOCKETSERVICE_HPP
