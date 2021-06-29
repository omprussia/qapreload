// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "LocalSocketClient.hpp"

#include <QLocalSocket>
#include <QDebug>

LocalSocketClient::LocalSocketClient(QLocalSocket *socket, QObject *parent)
    : ITransportClient(parent)
    , m_socket(socket)
{
    connect(m_socket, &QLocalSocket::readyRead, this, &LocalSocketClient::onReadyRead);
    connect(m_socket, &QLocalSocket::disconnected, this, &LocalSocketClient::onDisconnected);
    connect(m_socket, &QLocalSocket::connected, this, &LocalSocketClient::onConnected);
    connect(m_socket, static_cast<void (QLocalSocket::*)(QLocalSocket::LocalSocketError)>(&QLocalSocket::error),
        [](QLocalSocket::LocalSocketError error) mutable
        {
            qDebug() << Q_FUNC_INFO << error;
        });
    qDebug()
        << Q_FUNC_INFO
        << "New connection for:" << m_socket << endl
        << "state:" << m_socket->state();
    if (m_socket->state() != QLocalSocket::ConnectedState) {
        m_socket->waitForConnected();
    }
    qDebug()
        << Q_FUNC_INFO
        << "available:" << m_socket->bytesAvailable();
}

qint64 LocalSocketClient::bytesAvailable()
{
    return m_socket->bytesAvailable();
}

QByteArray LocalSocketClient::readAll()
{
    return m_socket->readAll();
}

bool LocalSocketClient::isOpen()
{
    return m_socket->isOpen();
}

bool LocalSocketClient::isConnected()
{
    return m_socket->state() == QLocalSocket::ConnectedState;
}

void LocalSocketClient::close()
{
    m_socket->close();
}

qint64 LocalSocketClient::write(const QByteArray &data)
{
    return m_socket->write(data);
}

bool LocalSocketClient::flush()
{
    return m_socket->flush();
}

bool LocalSocketClient::waitForBytesWritten(int msecs)
{
    return m_socket->waitForBytesWritten(msecs);
}

bool LocalSocketClient::waitForReadyRead(int msecs)
{
    return m_socket->waitForReadyRead(msecs);
}

void LocalSocketClient::onDisconnected()
{
    emit disconnected(this);
}

void LocalSocketClient::onConnected()
{
    emit connected(this);
}

void LocalSocketClient::onReadyRead()
{
    emit readyRead(this);
}
