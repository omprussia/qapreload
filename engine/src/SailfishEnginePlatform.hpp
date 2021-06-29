// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include "QuickEnginePlatform.hpp"

class SailfishEnginePlatform : public QuickEnginePlatform
{
    Q_OBJECT
public:
    explicit SailfishEnginePlatform(QWindow *window);

    enum SwipeDirection {
        SwipeDirectionUp,
        SwipeDirectionLeft,
        SwipeDirectionRight,
        SwipeDirectionDown,
    };
    Q_ENUM(SwipeDirection)

    enum PeekDirection {
        PeekDirectionUp,
        PeekDirectionLeft,
        PeekDirectionRight,
        PeekDirectionDown,
    };
    Q_ENUM(PeekDirection)

public slots:
    virtual void initialize() override;

private slots:
    // IEnginePlatform interface
    virtual void activateAppCommand(ITransportClient *socket, const QString &appName) override;
    virtual void queryAppStateCommand(ITransportClient *socket, const QString &appName) override;
    virtual void backCommand(ITransportClient *socket) override;
    virtual void forwardCommand(ITransportClient *socket) override;
    virtual void getOrientationCommand(ITransportClient *socket) override;
    virtual void setOrientationCommand(ITransportClient *socket, const QString &orientation) override;
    virtual void hideKeyboardCommand(ITransportClient *socket, const QString &strategy, const QString &key, double keyCode, const QString &keyName) override;
    virtual void isKeyboardShownCommand(ITransportClient *socket) override;
    virtual void activateIMEEngineCommand(ITransportClient *socket, const QVariant &engine) override;
    virtual void availableIMEEnginesCommand(ITransportClient *socket) override;
    virtual void getActiveIMEEngineCommand(ITransportClient *socket) override;
    virtual void deactivateIMEEngineCommand(ITransportClient *socket) override;
    virtual void isIMEActivatedCommand(ITransportClient *socket) override;

    // execute_%1 methods
    void executeCommand_app_pullDownTo(ITransportClient *socket, const QString &destination);
    void executeCommand_app_pullDownTo(ITransportClient *socket, double destination);
    void executeCommand_app_pushUpTo(ITransportClient *socket, const QString &destination);
    void executeCommand_app_pushUpTo(ITransportClient *socket, double destination);
    void executeCommand_app_clickContextMenuItem(ITransportClient *socket, const QString &elementId, const QString &destination);
    void executeCommand_app_clickContextMenuItem(ITransportClient *socket, const QString &elementId, double destination);
    void executeCommand_app_waitForPageChange(ITransportClient *socket, double timeout = 3000);
    void executeCommand_app_swipe(ITransportClient *socket, const QString &directionString);
    void executeCommand_app_peek(ITransportClient *socket, const QString &directionString);
    void executeCommand_app_goBack(ITransportClient *socket);
    void executeCommand_app_goForward(ITransportClient *socket);
    void executeCommand_app_enterCode(ITransportClient *socket, const QString &code);
    void executeCommand_app_scrollToItem(ITransportClient *socket, const QString &elementId);
    void executeCommand_app_saveScreenshot(ITransportClient *socket, const QString &fileName);
    void executeCommand_app_dumpCurrentPage(ITransportClient *socket);
    void executeCommand_app_dumpCover(ITransportClient *socket);

    // findElement_%1 methods
    void findStrategy_classname(ITransportClient *socket, const QString &selector, bool multiple = false, QObject *parentItem = nullptr);

    // own methods
    void onChildrenChanged();
    void getScreenshotCoverCommand(ITransportClient *socket);

    void bridgeStatusChanged(const QString &interface, const QVariantMap &properties, const QStringList &);

private:
    QQuickItem *getCoverItem();
    QQuickItem *getPageStack();
    QQuickItem *getCurrentPage();

    void pullDownTo(const QString &text);
    void pullDownTo(int index);
    void pushUpTo(const QString &text);
    void pushUpTo(int index);
    void scrollToItem(QQuickItem *item);
    QObjectList openContextMenu(QQuickItem *item);
    void clickContextMenuItem(QQuickItem *item, const QString &text, bool partial = true);
    void clickContextMenuItem(QQuickItem *item, int index);
    void waitForPageChange(int timeout = 1000);
    void swipe(SwipeDirection direction);
    void peek(PeekDirection direction);
    void enterCode(const QString &code);
    void goBack();
    void goForward();
};

