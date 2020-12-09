// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAEngineSocketClient.hpp"
#include "QAEngine.hpp"

#include <QDebug>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTcpSocket>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(categorySocketClient, "omp.qaengine.socket", QtInfoMsg)

QAEngineSocketClient::QAEngineSocketClient(QObject *parent)
    : QObject(parent)
{

}

void QAEngineSocketClient::connectToBridge()
{
    qCDebug(categorySocketClient)
        << Q_FUNC_INFO;

    if (!m_socket) {
        m_socket = new QTcpSocket(this);
    }

    m_socket->connectToHost(QHostAddress(QHostAddress::LocalHost), 8888);
    m_socket->waitForConnected();
    if (!m_socket->isOpen()) {
        qCWarning(categorySocketClient)
            << Q_FUNC_INFO
            << "Can't connect to bridge socket";
        m_socket->deleteLater();
        m_socket = nullptr;
        return;
    }

    QJsonObject root;
    QJsonObject app;
    app.insert(QStringLiteral("appName"), QAEngine::processName());

    root.insert(QStringLiteral("appConnect"), app);

    QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Compact);

    qCDebug(categorySocketClient)
        << Q_FUNC_INFO
        << "Writing to bridge socket:"
        << m_socket->write(data)
        << m_socket->waitForBytesWritten();

    connect(m_socket, &QTcpSocket::readyRead, this, &QAEngineSocketClient::readSocket, Qt::UniqueConnection);
}

void QAEngineSocketClient::readSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    qCDebug(categorySocketClient)
        << Q_FUNC_INFO
        << socket << socket->bytesAvailable();

    emit commandReceived(socket, socket->readAll());
}
