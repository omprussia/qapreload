// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include <QHash>
#include <QObject>

class ITransportClient;
class ITransportServer : public QObject
{
    Q_OBJECT
public:
    explicit ITransportServer(QObject *parent = nullptr) : QObject(parent) {}


    virtual void registerClient(ITransportClient *client);
    virtual void readData(ITransportClient *client);

public slots:
    virtual void start() = 0;

signals:
    void commandReceived(ITransportClient *client, const QByteArray &cmd);
    void clientLost(ITransportClient *client);

private:
    QHash<QObject*, ITransportClient*> m_clients;
};
