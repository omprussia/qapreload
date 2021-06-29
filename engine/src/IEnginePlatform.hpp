// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once
#include <QObject>
#include <QVariant>
#include <QPoint>
#include <QSize>
#include <QRect>
#include <QWindow>

class ITransportClient;
class IEnginePlatform : public QObject
{
    Q_OBJECT
public:
    explicit IEnginePlatform(QWindow* window) : QObject(window) {}
    virtual QWindow* window() = 0;
    virtual QObject* rootObject() = 0;

    virtual void socketReply(ITransportClient *socket, const QVariant &value, int status = 0) = 0;
    virtual void elementReply(ITransportClient *socket, QObjectList elements, bool multiple = false) = 0;

    virtual void addItem(QObject *o) = 0;
    virtual void removeItem(QObject *o) = 0;

    virtual bool containsObject(const QString &elementId) = 0;
    virtual QObject *getObject(const QString &elementId) = 0;
    virtual QObject *getParent(QObject *item) = 0;
    virtual QString getText(QObject *item) = 0;
    virtual QPoint getAbsPosition(QObject *item) = 0;
    virtual QPoint getPosition(QObject *item) = 0;
    virtual QSize getSize(QObject *item) = 0;
    virtual QRect getGeometry(QObject *item) = 0;
    virtual QRect getAbsGeometry(QObject *item) = 0;
    virtual bool isItemEnabled(QObject *item) = 0;
    virtual bool isItemVisible(QObject *item) = 0;

public slots:
    virtual void initialize() = 0;

private slots:
    virtual void activateAppCommand(ITransportClient *socket, const QString &appName) = 0;
    virtual void closeAppCommand(ITransportClient *socket, const QString &appName) = 0;
    virtual void queryAppStateCommand(ITransportClient *socket, const QString &appName) = 0;
    virtual void backgroundCommand(ITransportClient *socket, double seconds) = 0;
    virtual void getClipboardCommand(ITransportClient *socket) = 0;
    virtual void setClipboardCommand(ITransportClient *socket, const QString &content) = 0;
    virtual void findElementCommand(ITransportClient *socket, const QString &strategy, const QString &selector) = 0;
    virtual void findElementsCommand(ITransportClient *socket, const QString &strategy, const QString &selector) = 0;
    virtual void findElementFromElementCommand(ITransportClient *socket, const QString &strategy, const QString &selector, const QString &elementId) = 0;
    virtual void findElementsFromElementCommand(ITransportClient *socket, const QString &strategy, const QString &selector, const QString &elementId) = 0;
    virtual void getLocationCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void getLocationInViewCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void getAttributeCommand(ITransportClient *socket, const QString &attribute, const QString &elementId) = 0;
    virtual void getPropertyCommand(ITransportClient *socket, const QString &attribute, const QString &elementId) = 0;
    virtual void getTextCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void getElementScreenshotCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void getScreenshotCommand(ITransportClient *socket) = 0;
    virtual void getWindowRectCommand(ITransportClient *socket) = 0;
    virtual void elementEnabledCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void elementDisplayedCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void elementSelectedCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void getSizeCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void setValueImmediateCommand(ITransportClient *socket, const QVariantList &value, const QString &elementId) = 0;
    virtual void replaceValueCommand(ITransportClient *socket, const QVariantList &value, const QString &elementId) = 0;
    virtual void setValueCommand(ITransportClient *socket, const QVariantList &value, const QString &elementId) = 0;
    virtual void clickCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void clearCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void submitCommand(ITransportClient *socket, const QString &elementId) = 0;
    virtual void getPageSourceCommand(ITransportClient *socket) = 0;
    virtual void backCommand(ITransportClient *socket) = 0;
    virtual void forwardCommand(ITransportClient *socket) = 0;
    virtual void getOrientationCommand(ITransportClient *socket) = 0;
    virtual void setOrientationCommand(ITransportClient *socket, const QString &orientation) = 0;
    virtual void hideKeyboardCommand(ITransportClient *socket, const QString &strategy, const QString &key = QString(), double keyCode = 0, const QString &keyName = QString()) = 0;
    virtual void getCurrentActivityCommand(ITransportClient *socket) = 0;
    virtual void implicitWaitCommand(ITransportClient *socket, double msecs) = 0; // ?
    virtual void activeCommand(ITransportClient *socket) = 0; //for functions switch_to.active_element
    virtual void getAlertTextCommand(ITransportClient *socket) = 0; //for functions switch_to.alert
    virtual void isKeyboardShownCommand(ITransportClient *socket) = 0;
    virtual void activateIMEEngineCommand(ITransportClient *socket, const QVariant &engine) = 0;
    virtual void availableIMEEnginesCommand(ITransportClient *socket) = 0;
    virtual void getActiveIMEEngineCommand(ITransportClient *socket) = 0;
    virtual void deactivateIMEEngineCommand(ITransportClient *socket) = 0;
    virtual void isIMEActivatedCommand(ITransportClient *socket) = 0;
    virtual void keyeventCommand(ITransportClient *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &sessionIDArg, const QVariant &flagsArg) = 0;
    virtual void longPressKeyCodeCommand(ITransportClient *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg) = 0;
    virtual void pressKeyCodeCommand(ITransportClient *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg) = 0;
    virtual void executeCommand(ITransportClient *socket, const QString &command, const QVariantList &params) = 0;
    virtual void executeAsyncCommand(ITransportClient *socket, const QString &command, const QVariantList &params) = 0;
    virtual void performTouchCommand(ITransportClient *socket, const QVariant &paramsArg) = 0;
    virtual void performMultiActionCommand(ITransportClient *socket, const QVariant &paramsArg) = 0;
    virtual void performActionsCommand(ITransportClient *socket, const QVariant &paramsArg) = 0;

signals:
    void ready();
};
