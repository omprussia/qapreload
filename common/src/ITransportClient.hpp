// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include <QObject>

class ITransportClient : public QObject
{
    Q_OBJECT
public:
    explicit ITransportClient(QObject *parent = nullptr) : QObject(parent) {}

    virtual qint64 bytesAvailable() = 0;
    virtual QByteArray readAll() = 0;

    virtual bool isOpen() = 0;
    virtual bool isConnected() = 0;
    virtual void close() = 0;

    virtual qint64 write(const QByteArray &data) = 0;
    virtual bool flush() = 0;

    virtual bool waitForBytesWritten(int msecs = 30000) = 0;
    virtual bool waitForReadyRead(int msecs = 30000) = 0;

signals:
    void readyRead(ITransportClient *client);
    void connected(ITransportClient *client);
    void disconnected(ITransportClient *client);
};
