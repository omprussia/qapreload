#pragma once

#include "IEnginePlatform.hpp"

class QAMouseEngine;
class QAKeyEngine;
class QTouchEvent;
class QMouseEvent;
class QKeyEvent;
class QWindow;
class GenericEnginePlatform : public IEnginePlatform
{
public:
    explicit GenericEnginePlatform(QObject *parent);
    void socketReply(QTcpSocket *socket, const QVariant &value, int status = 0) override;
    void elementReply(QTcpSocket *socket, const QVariantList &elements, bool multiple = false) override;

    static QString getClassName(QObject *item);
    static QString uniqueId(QObject *item);

    QObject *getItem(const QString &elementId);

public slots:
    virtual void initialize() = 0;

protected:
    void clickPoint(int posx, int posy);
    void pressAndHold(int posx, int posy, int delay = 800);
    void mouseMove(int startx, int starty, int stopx, int stopy);
    void mouseDrag(int startx, int starty, int stopx, int stopy, int delay = 1200);

    template <typename T>
    T getItem(const QString &elementId);

    QWindow *m_rootWindow = nullptr;
    QObject *m_rootObject = nullptr;

    QHash<QString, QObject*> m_items;
    QAMouseEngine *m_mouseEngine;
    QAKeyEngine *m_keyEngine;

private:
    void execute(QTcpSocket *socket, const QString &methodName, const QVariantList &params);

private slots:
    // own stuff
    void onPropertyChanged();

    // synthesized input events
    virtual void onTouchEvent(const QTouchEvent &event);
    virtual void onMouseEvent(const QMouseEvent &event);
    virtual void onKeyEvent(QKeyEvent *event);

    // IEnginePlatform interface
    virtual void activateAppCommand(QTcpSocket *socket, const QString &appName) override;
    virtual void closeAppCommand(QTcpSocket *socket, const QString &appName) override;
    virtual void queryAppStateCommand(QTcpSocket *socket, const QString &appName) override;
    virtual void backgroundCommand(QTcpSocket *socket, double seconds) override;
    virtual void getClipboardCommand(QTcpSocket *socket) override;
    virtual void setClipboardCommand(QTcpSocket *socket, const QString &content) override;
    virtual void findElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector) override;
    virtual void findElementsCommand(QTcpSocket *socket, const QString &strategy, const QString &selector) override;
    virtual void findElementFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId) override;
    virtual void findElementsFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId) override;
    virtual void getLocationCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getLocationInViewCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getAttributeCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId) override;
    virtual void getPropertyCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId) override;
    virtual void getTextCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getElementScreenshotCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getScreenshotCommand(QTcpSocket *socket) override;
    virtual void elementEnabledCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void elementDisplayedCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void elementSelectedCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getSizeCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void setValueImmediateCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId) override;
    virtual void replaceValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId) override;
    virtual void setValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId) override;
    virtual void clickCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void clearCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void submitCommand(QTcpSocket *socket, const QString &elementId) override;
    virtual void getPageSourceCommand(QTcpSocket *socket) override;
    virtual void backCommand(QTcpSocket *socket) override;
    virtual void forwardCommand(QTcpSocket *socket) override;
    virtual void getOrientationCommand(QTcpSocket *socket) override;
    virtual void setOrientationCommand(QTcpSocket *socket, const QString &orientation) override;
    virtual void hideKeyboardCommand(QTcpSocket *socket, const QString &strategy, const QString &key, double keyCode, const QString &keyName) override;
    virtual void getCurrentActivityCommand(QTcpSocket *socket) override;
    virtual void implicitWaitCommand(QTcpSocket *socket, double msecs) override;
    virtual void activeCommand(QTcpSocket *socket) override;
    virtual void getAlertTextCommand(QTcpSocket *socket) override;
    virtual void isKeyboardShownCommand(QTcpSocket *socket) override;
    virtual void activateIMEEngineCommand(QTcpSocket *socket, const QVariant &engine) override;
    virtual void availableIMEEnginesCommand(QTcpSocket *socket) override;
    virtual void getActiveIMEEngineCommand(QTcpSocket *socket) override;
    virtual void deactivateIMEEngineCommand(QTcpSocket *socket) override;
    virtual void isIMEActivatedCommand(QTcpSocket *socket) override;
    virtual void keyeventCommand(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &sessionIDArg, const QVariant &flagsArg) override;
    virtual void longPressKeyCodeCommand(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg) override;
    virtual void pressKeyCodeCommand(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg) override;
    virtual void executeCommand(QTcpSocket *socket, const QString &command, const QVariantList &params) override;
    virtual void executeAsyncCommand(QTcpSocket *socket, const QString &command, const QVariantList &params) override;

    // execute_%1 methods
    void executeCommand_app_waitForPropertyChange(QTcpSocket *socket, const QString &elementId, const QString &propertyName, const QVariant &value, double timeout = 3000);
};

