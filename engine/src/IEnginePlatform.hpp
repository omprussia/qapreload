// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once
#include <QObject>
#include <QVariant>
#include <QPoint>
#include <QSize>
#include <QRect>

class QTcpSocket;
class IEnginePlatform : public QObject
{
    Q_OBJECT
public:
    explicit IEnginePlatform(QObject* parent) : QObject(parent) {}
    virtual void socketReply(QTcpSocket *socket, const QVariant &value, int status = 0) = 0;
    virtual void elementReply(QTcpSocket *socket, QObjectList elements, bool multiple = false) = 0;

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

private slots:
    virtual void activateAppCommand(QTcpSocket *socket, const QString &appName) = 0;
    virtual void closeAppCommand(QTcpSocket *socket, const QString &appName) = 0;
    virtual void queryAppStateCommand(QTcpSocket *socket, const QString &appName) = 0;
    virtual void backgroundCommand(QTcpSocket *socket, double seconds) = 0;
    virtual void getClipboardCommand(QTcpSocket *socket) = 0;
    virtual void setClipboardCommand(QTcpSocket *socket, const QString &content) = 0;
    virtual void findElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector) = 0;
    virtual void findElementsCommand(QTcpSocket *socket, const QString &strategy, const QString &selector) = 0;
    virtual void findElementFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId) = 0;
    virtual void findElementsFromElementCommand(QTcpSocket *socket, const QString &strategy, const QString &selector, const QString &elementId) = 0;
    virtual void getLocationCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void getLocationInViewCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void getAttributeCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId) = 0;
    virtual void getPropertyCommand(QTcpSocket *socket, const QString &attribute, const QString &elementId) = 0;
    virtual void getTextCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void getElementScreenshotCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void getScreenshotCommand(QTcpSocket *socket) = 0;
    virtual void elementEnabledCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void elementDisplayedCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void elementSelectedCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void getSizeCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void setValueImmediateCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId) = 0;
    virtual void replaceValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId) = 0;
    virtual void setValueCommand(QTcpSocket *socket, const QVariantList &value, const QString &elementId) = 0;
    virtual void clickCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void clearCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void submitCommand(QTcpSocket *socket, const QString &elementId) = 0;
    virtual void getPageSourceCommand(QTcpSocket *socket) = 0;
    virtual void backCommand(QTcpSocket *socket) = 0;
    virtual void forwardCommand(QTcpSocket *socket) = 0;
    virtual void getOrientationCommand(QTcpSocket *socket) = 0;
    virtual void setOrientationCommand(QTcpSocket *socket, const QString &orientation) = 0;
    virtual void hideKeyboardCommand(QTcpSocket *socket, const QString &strategy, const QString &key = QString(), double keyCode = 0, const QString &keyName = QString()) = 0;
    virtual void getCurrentActivityCommand(QTcpSocket *socket) = 0;
    virtual void implicitWaitCommand(QTcpSocket *socket, double msecs) = 0; // ?
    virtual void activeCommand(QTcpSocket *socket) = 0; //for functions switch_to.active_element
    virtual void getAlertTextCommand(QTcpSocket *socket) = 0; //for functions switch_to.alert
    virtual void isKeyboardShownCommand(QTcpSocket *socket) = 0;
    virtual void activateIMEEngineCommand(QTcpSocket *socket, const QVariant &engine) = 0;
    virtual void availableIMEEnginesCommand(QTcpSocket *socket) = 0;
    virtual void getActiveIMEEngineCommand(QTcpSocket *socket) = 0;
    virtual void deactivateIMEEngineCommand(QTcpSocket *socket) = 0;
    virtual void isIMEActivatedCommand(QTcpSocket *socket) = 0;
    virtual void keyeventCommand(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &sessionIDArg, const QVariant &flagsArg) = 0;
    virtual void longPressKeyCodeCommand(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg) = 0;
    virtual void pressKeyCodeCommand(QTcpSocket *socket, const QVariant &keycodeArg, const QVariant &metaState, const QVariant &flagsArg) = 0;
    virtual void executeCommand(QTcpSocket *socket, const QString &command, const QVariantList &params) = 0;
    virtual void executeAsyncCommand(QTcpSocket *socket, const QString &command, const QVariantList &params) = 0;
    virtual void performTouchCommand(QTcpSocket *socket, const QVariant &paramsArg) = 0;
    virtual void performMultiActionCommand(QTcpSocket *socket, const QVariant &paramsArg) = 0;

signals:
    void ready();
};
