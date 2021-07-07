// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include <QObject>

class ITransportClient;
class QAEngineSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit QAEngineSocketClient(QObject *parent = nullptr);

signals:
    void commandReceived(ITransportClient *socket, const QByteArray &cmd);

public slots:
    void connectToBridge();

private slots:
    void readClient(ITransportClient *client);
    void onConnected();

private:
    ITransportClient *m_client = nullptr;
};

