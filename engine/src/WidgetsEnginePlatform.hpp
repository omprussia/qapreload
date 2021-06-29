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
    explicit WidgetsEnginePlatform(QWindow *window);
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

    void grabScreenshot(ITransportClient *socket, QObject *item, bool fillBackground = false) override;

    void pressAndHoldItem(QObject *qitem, int delay = 800) override;

    QHash<QObject*, QWidget*> m_rootWidgets;
    QWidget *m_rootWidget = nullptr;

private slots:
    // IEnginePlatform interface
    virtual void getPageSourceCommand(ITransportClient *socket) override;

    // execute_%1 methods
    void executeCommand_app_dumpInView(ITransportClient *socket, const QString &elementId);
    void executeCommand_app_posInView(ITransportClient *socket, const QString &elementId, const QString &display);
    void executeCommand_app_clickInView(ITransportClient *socket, const QString &elementId, const QString &display);
    void executeCommand_app_scrollInView(ITransportClient *socket, const QString &elementId, const QString &display);
    void executeCommand_app_triggerInMenu(ITransportClient *socket, const QString &text);
    void executeCommand_app_dumpInMenu(ITransportClient *socket);
    void executeCommand_app_dumpInComboBox(ITransportClient *socket, const QString &elementId);
    void executeCommand_app_activateInComboBox(ITransportClient *socket, const QString &elementId, const QString &display);
    void executeCommand_app_activateInComboBox(ITransportClient *socket, const QString &elementId, double idx);
    void executeCommand_app_dumpInTabBar(ITransportClient *socket, const QString &elementId);
    void executeCommand_app_posInTabBar(ITransportClient *socket, const QString &elementId, const QString &display);
    void executeCommand_app_posInTabBar(ITransportClient *socket, const QString &elementId, double idx);
    void executeCommand_app_activateInTabBar(ITransportClient *socket, const QString &elementId, const QString &display);
    void executeCommand_app_activateInTabBar(ITransportClient *socket, const QString &elementId, double idx);
private:
    QModelIndex recursiveFindModel(QAbstractItemModel *model, QModelIndex index, const QString &display, bool partial = false);
    QStringList recursiveDumpModel(QAbstractItemModel *model, QModelIndex index);
};

class EventHandler : public QObject
{
    Q_OBJECT
public:
    explicit EventHandler(QObject *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event) override;
};

