// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "LocalSocketClient.hpp"
#include "LocalSocketServer.hpp"

#include <QFile>
#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>

#include <QDebug>

LocalSocketServer::LocalSocketServer(const QString &name, QObject *parent)
    : ITransportServer(parent)
    , m_name(name)
    , m_server(new QLocalServer(this))
{
    connect(m_server, &QLocalServer::newConnection, this, &LocalSocketServer::newConnection);

    m_server->setSocketOptions(QLocalServer::WorldAccessOption);
    m_server->setMaxPendingConnections(2147483647);
}

void LocalSocketServer::start()
{
    qDebug() << Q_FUNC_INFO;

    bool listening = m_server->listen(m_name);
    if (!listening // not listening
            && m_server->serverError() == QAbstractSocket::AddressInUseError // because of AddressInUseError
            && QFileInfo::exists(m_name) // socket file already exists
            && QFile::remove(m_name)) { // and successfully removed it
        qWarning()
            << Q_FUNC_INFO
            << "Removed old stuck socket";
        listening = m_server->listen(m_name); // try to start lisening again
    }
    qDebug()
        << Q_FUNC_INFO
        << "Server listening:" << listening;
    if (!listening) {
        qWarning()
            << Q_FUNC_INFO
            << "Server error:" << m_server->serverError() << m_server->errorString();
    }
}

void LocalSocketServer::newConnection()
{
    auto socket = m_server->nextPendingConnection();
    auto client = new LocalSocketClient(socket);
    registerClient(client);
}
