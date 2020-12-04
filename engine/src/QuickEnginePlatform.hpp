// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include "GenericEnginePlatform.hpp"

#include <QJsonObject>

class QQmlEngine;
class QQuickItem;
class QQuickWindow;
class QXmlStreamWriter;
class QuickEnginePlatform : public GenericEnginePlatform
{
    Q_OBJECT
public:
    explicit QuickEnginePlatform(QObject *parent);
    QQuickItem *getItem(const QString &elementId);

public slots:
    virtual void initialize() override;

protected:
    QList<QObject*> childrenList(QObject *parentItem) override;

    QQuickItem *findParentFlickable(QQuickItem *rootItem = nullptr);
    QVariantList findNestedFlickable(QQuickItem *parentItem = nullptr);
    QQuickItem *getApplicationWindow();

    QObject *getParent(QObject *item) override;
    QPoint getAbsPosition(QObject *item) override;
    QPoint getPosition(QObject *item) override;
    QSize getSize(QObject *item) override;
    bool isItemEnabled(QObject *item) override;
    bool isItemVisible(QObject *item) override;

    QVariant executeJS(const QString &jsCode, QQuickItem *item);

    void grabScreenshot(QTcpSocket *socket, QObject *item, bool fillBackground = false) override;

    void pressAndHoldItem(QObject *qitem, int delay = 800) override;
    void clearFocus();
    void clearComponentCache();

    QQmlEngine *getEngine(QQuickItem *item = nullptr);

    QQuickItem *m_rootQuickItem = nullptr;
    QQuickWindow *m_rootQuickWindow = nullptr;

private slots:
    void onFocusWindowChanged(QWindow *window);

    // IEnginePlatform interface
    virtual void getPageSourceCommand(QTcpSocket *socket) override;

    // synthesized input events
    virtual void onKeyEvent(QKeyEvent *event) override;

    // execute_%1 methods
    void executeCommand_touch_pressAndHold(QTcpSocket *socket, double posx, double posy);
    void executeCommand_touch_mouseSwipe(QTcpSocket *socket, double posx, double posy, double stopx, double stopy);
    void executeCommand_touch_mouseDrag(QTcpSocket *socket, double posx, double posy, double stopx, double stopy);
    void executeCommand_app_method(QTcpSocket *socket, const QString &elementId, const QString &method, const QVariantList &params);
    void executeCommand_app_js(QTcpSocket *socket, const QString &elementId, const QString &jsCode);
};

