// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "TCPSocketClient.hpp"
#include "TCPSocketServer.hpp"

#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>

#include <QDebug>

TCPSocketServer::TCPSocketServer(quint16 port, QObject *parent)
    : ITransportServer(parent)
    , m_port(port)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &TCPSocketServer::newConnection);
}

void TCPSocketServer::start()
{
    qDebug() << Q_FUNC_INFO;

#ifdef USE_SYSTEMD
    if (sd_listen_fds(0) == 1) {
        int fd = SD_LISTEN_FDS_START;
        qDebug()
            << Q_FUNC_INFO
            << "Using systemd socket descriptor:" << fd
            << m_server->setSocketDescriptor(fd);
    } else
#endif
    if (!m_server->listen(QHostAddress::AnyIPv4, m_port)) {
        qWarning()
            << Q_FUNC_INFO
            << m_server->errorString();
        qApp->quit();
        return;
    }
}

void TCPSocketServer::newConnection()
{
    auto socket = m_server->nextPendingConnection();
    qDebug()
        << Q_FUNC_INFO
        << "New connection from:" << socket->peerAddress() << socket->peerPort();
    auto client = new TCPSocketClient(socket);
    registerClient(client);
}
