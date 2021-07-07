// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once
#include "ITransportClient.hpp"

class QLocalSocket;
class LocalSocketClient : public ITransportClient
{
    Q_OBJECT
public:
    explicit LocalSocketClient(QLocalSocket *socket, QObject *parent = nullptr);
    qint64 bytesAvailable() override;
    QByteArray readAll() override;
    bool isOpen() override;
    bool isConnected() override;
    void close() override;
    qint64 write(const QByteArray &data) override;
    bool flush() override;
    bool waitForBytesWritten(int msecs) override;
    bool waitForReadyRead(int msecs) override;

private slots:
    void onDisconnected();
    void onConnected();
    void onReadyRead();

private:
    QLocalSocket *m_socket = nullptr;
};
