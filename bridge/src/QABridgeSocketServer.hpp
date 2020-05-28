// Copyright (c) 2020 Open Mobile Platform LLС.
#pragma once
#include <QHash>
#include <QObject>

class QTcpServer;
class QTcpSocket;
class QABridgeSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit QABridgeSocketServer(QObject *parent = nullptr);

signals:
    void commandReceived(QTcpSocket *socket, const QByteArray &cmd);
    void clientLost(QTcpSocket *socket);

public slots:
    void start();

private slots:
    void newConnection();
    void readSocket();
    void removeSocket();

private:
    QTcpServer *m_server = nullptr;
    QHash<QTcpSocket*, QString> m_clientSocket;
};

