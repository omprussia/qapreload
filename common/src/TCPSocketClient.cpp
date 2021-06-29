// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "TCPSocketClient.hpp"

#include <QDebug>
#include <QTcpSocket>

TCPSocketClient::TCPSocketClient(QTcpSocket *socket, QObject *parent)
    : ITransportClient(parent)
    , m_socket(socket)
{
    connect(m_socket, &QTcpSocket::readyRead, this, &TCPSocketClient::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &TCPSocketClient::onDisconnected);
}

qint64 TCPSocketClient::bytesAvailable()
{
    return m_socket->bytesAvailable();
}

QByteArray TCPSocketClient::readAll()
{
    return m_socket->readAll();
}

bool TCPSocketClient::isOpen()
{
    return m_socket->isOpen();
}

bool TCPSocketClient::isConnected()
{
    return m_socket->state() == QTcpSocket::ConnectedState;
}

void TCPSocketClient::close()
{
    m_socket->close();
}

qint64 TCPSocketClient::write(const QByteArray &data)
{
    return m_socket->write(data);
}

bool TCPSocketClient::flush()
{
    return m_socket->flush();
}

bool TCPSocketClient::waitForBytesWritten(int msecs)
{
    return m_socket->waitForBytesWritten(msecs);
}

bool TCPSocketClient::waitForReadyRead(int msecs)
{
    return m_socket->waitForReadyRead(msecs);
}

void TCPSocketClient::onDisconnected()
{
    emit disconnected(this);
}

void TCPSocketClient::onConnected()
{
    emit connected(this);
}

void TCPSocketClient::onReadyRead()
{
    emit readyRead(this);
}
