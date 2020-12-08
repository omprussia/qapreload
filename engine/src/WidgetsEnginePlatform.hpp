// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include "GenericEnginePlatform.hpp"

#include <QJsonObject>
#include <QModelIndex>

class QAbstractItemModel;
class QAbstractItemView;
class QWidget;
class QXmlStreamWriter;
class WidgetsEnginePlatform : public GenericEnginePlatform
{
    Q_OBJECT
public:
    explicit WidgetsEnginePlatform(QObject *parent);
    QWidget *getItem(const QString &elementId);

public slots:
    virtual void initialize() override;\

    QObject *getParent(QObject *item) override;

    QPoint getAbsPosition(QObject *item) override;
    QPoint getPosition(QObject *item) override;
    QSize getSize(QObject *item) override;
    bool isItemEnabled(QObject *item) override;
    bool isItemVisible(QObject *item) override;

protected:
    QList<QObject*> childrenList(QObject *parentItem) override;

    void grabScreenshot(QTcpSocket *socket, QObject *item, bool fillBackground = false) override;

    void pressAndHoldItem(QObject *qitem, int delay = 800) override;

    QHash<QObject*, QWidget*> m_rootWidgets;
    QWidget *m_rootWidget = nullptr;

private slots:
    void onFocusWindowChanged(QWindow *window);

    // IEnginePlatform interface
    virtual void getPageSourceCommand(QTcpSocket *socket) override;

    // execute_%1 methods
    void executeCommand_app_dumpInView(QTcpSocket *socket, const QString &elementId);
    void executeCommand_app_posInView(QTcpSocket *socket, const QString &elementId, const QString &display);
    void executeCommand_app_clickInView(QTcpSocket *socket, const QString &elementId, const QString &display);
    void executeCommand_app_scrollInView(QTcpSocket *socket, const QString &elementId, const QString &display);
    void executeCommand_app_triggerInMenu(QTcpSocket *socket, const QString &text);
    void executeCommand_app_dumpInMenu(QTcpSocket *socket);
    void executeCommand_app_dumpInComboBox(QTcpSocket *socket, const QString &elementId);
    void executeCommand_app_activateInComboBox(QTcpSocket *socket, const QString &elementId, const QString &display);
    void executeCommand_app_activateInComboBox(QTcpSocket *socket, const QString &elementId, double idx);
    void executeCommand_app_dumpInTabBar(QTcpSocket *socket, const QString &elementId);
    void executeCommand_app_posInTabBar(QTcpSocket *socket, const QString &elementId, const QString &display);
    void executeCommand_app_posInTabBar(QTcpSocket *socket, const QString &elementId, double idx);
    void executeCommand_app_activateInTabBar(QTcpSocket *socket, const QString &elementId, const QString &display);
    void executeCommand_app_activateInTabBar(QTcpSocket *socket, const QString &elementId, double idx);
private:
    QModelIndex recursiveFindModel(QAbstractItemModel *model, QModelIndex index, const QString &display, bool partial = false);
    QStringList recursiveDumpModel(QAbstractItemModel *model, QModelIndex index);
};

