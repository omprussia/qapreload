#pragma once

#include "GenericEnginePlatform.hpp"

#include <QJsonObject>

class QWidget;
class QXmlStreamWriter;
class WidgetsEnginePlatform : public GenericEnginePlatform
{
    Q_OBJECT
public:
    explicit WidgetsEnginePlatform(QObject *parent);
    QWidget *getItem(const QString &elementId);

public slots:
    virtual void initialize() override;

protected:
    QList<QObject*> childrenList(QObject *parentItem) override;
    QObject *getParent(QObject *item) override;

    QPoint getAbsPosition(QObject *item) override;
    QPoint getPosition(QObject *item) override;
    QSize getSize(QObject *item) override;
    bool isItemEnabled(QObject *item) override;
    bool isItemVisible(QObject *item) override;

    void grabScreenshot(QTcpSocket *socket, QObject *item, bool fillBackground = false) override;

    void pressAndHoldItem(QObject *qitem, int delay = 800) override;

    QWidget *m_rootWidget = nullptr;

private slots:
    void onFocusWindowChanged(QWindow *window);

    // IEnginePlatform interface
    virtual void getPageSourceCommand(QTcpSocket *socket) override;
};

