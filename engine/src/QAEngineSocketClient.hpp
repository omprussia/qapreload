// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include <QObject>

class QTcpSocket;
class QAEngineSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit QAEngineSocketClient(QObject *parent = nullptr);

signals:
    void commandReceived(QTcpSocket *socket, const QByteArray &cmd);

public slots:
    void connectToBridge();

private slots:
    void readSocket();

private:
    QTcpSocket *m_socket = nullptr;
};

