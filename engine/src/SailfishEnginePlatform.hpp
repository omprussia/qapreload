#pragma once

#include "QuickEnginePlatform.hpp"

class SailfishEnginePlatform : public QuickEnginePlatform
{
    Q_OBJECT
public:
    explicit SailfishEnginePlatform(QObject *parent);

public slots:
    virtual void initialize() override;

private slots:
    // IEnginePlatform methods

    // execute_%1 methods
    void executeCommand_app_pullDownTo(QTcpSocket *socket, const QString &destination);
    void executeCommand_app_pullDownTo(QTcpSocket *socket, double destination);
    void executeCommand_app_pushUpTo(QTcpSocket *socket, const QString &destination);
    void executeCommand_app_pushUpTo(QTcpSocket *socket, double destination);

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
};

