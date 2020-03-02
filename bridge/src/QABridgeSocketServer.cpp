#include "QABridgeSocketServer.hpp"

#ifdef USE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>

#include <QDebug>

QABridgeSocketServer::QABridgeSocketServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &QABridgeSocketServer::newConnection);
}

void QABridgeSocketServer::start()
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
    if (!m_server->listen(QHostAddress::AnyIPv4, 8888)) {
        qWarning()
            << Q_FUNC_INFO
            << m_server->errorString();
        qApp->quit();
        return;
    }
}

void QABridgeSocketServer::newConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &QABridgeSocketServer::readSocket);
    connect(socket, &QTcpSocket::disconnected, this, &QABridgeSocketServer::removeSocket);
    qDebug()
        << Q_FUNC_INFO
        << "New connection from:" << socket->peerAddress() << socket->peerPort();
}

void QABridgeSocketServer::readSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    qDebug()
        << Q_FUNC_INFO
        << socket << socket->bytesAvailable();

    QByteArray requestData = socket->readAll();
    qDebug().noquote() << requestData;

    requestData.replace("}{", "}\n{"); // workaround packets join

    const QList<QByteArray> commands = requestData.split('\n');
    for (const QByteArray &cmd : commands) {
        if (cmd.isEmpty()) {
            continue;
        }

        qDebug().noquote() << cmd;
        emit commandReceived(socket, cmd);
    }
}

void QABridgeSocketServer::removeSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    qDebug()
        << Q_FUNC_INFO
        << socket;

    emit clientLost(socket);
}