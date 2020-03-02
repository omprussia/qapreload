#pragma once

#include "QuickEnginePlatform.hpp"

class SailfishEnginePlatform : public QuickEnginePlatform
{
    Q_OBJECT
public:
    explicit SailfishEnginePlatform(QObject *parent);

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
    // IEnginePlatform methods

    // execute_%1 methods
    void executeCommand_app_pullDownTo(QTcpSocket *socket, const QString &destination);
    void executeCommand_app_pullDownTo(QTcpSocket *socket, double destination);
    void executeCommand_app_pushUpTo(QTcpSocket *socket, const QString &destination);
    void executeCommand_app_pushUpTo(QTcpSocket *socket, double destination);
    void executeCommand_app_clickContextMenuItem(QTcpSocket *socket, const QString &elementId, const QString &destination);
    void executeCommand_app_clickContextMenuItem(QTcpSocket *socket, const QString &elementId, double destination);
    void executeCommand_app_waitForPageChange(QTcpSocket *socket, double timeout = 3000);
    void executeCommand_app_swipe(QTcpSocket *socket, const QString &directionString);
    void executeCommand_app_peek(QTcpSocket *socket, const QString &directionString);
    void executeCommand_app_goBack(QTcpSocket *socket);
    void executeCommand_app_goForward(QTcpSocket *socket);
    void executeCommand_app_enterCode(QTcpSocket *socket, const QString &code);
    void executeCommand_app_scrollToItem(QTcpSocket *socket, const QString &elementId);
    void executeCommand_app_saveScreenshot(QTcpSocket *socket, const QString &fileName);

    // own methods
    void onChildrenChanged();

private:
    QQuickItem *coverItem();
    QQuickItem *getPageStack();
    QQuickItem *getCurrentPage();

    void pullDownTo(const QString &text);
    void pullDownTo(int index);
    void pushUpTo(const QString &text);
    void pushUpTo(int index);
    void scrollToItem(QQuickItem *item);
    QVariantList openContextMenu(QQuickItem *item);
    void clickContextMenuItem(QQuickItem *item, const QString &text, bool partial = true);
    void clickContextMenuItem(QQuickItem *item, int index);
    void waitForPageChange(int timeout = 1000);
    void swipe(SwipeDirection direction);
    void peek(PeekDirection direction);
    void enterCode(const QString &code);
};

