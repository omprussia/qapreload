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

Q_LOGGING_CATEGORY(categorySocketClient, "omp.qaengine.socket", QtWarningMsg)

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

    if (m_socket->isOpen()) {
        return;
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
    auto bytes = m_socket->write(data);
    qCDebug(categorySocketClient)
        << Q_FUNC_INFO
        << "Bytes to write:"
        << bytes;
    auto success = m_socket->waitForBytesWritten();
    qCDebug(categorySocketClient)
        << "Writing to bridge socket:"
        << success;

    connect(m_socket, &QTcpSocket::readyRead, this, &QAEngineSocketClient::readSocket, Qt::UniqueConnection);
}

void QAEngineSocketClient::readSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    auto bytes = socket->bytesAvailable();
    qCDebug(categorySocketClient)
        << Q_FUNC_INFO
        << socket << bytes;

    emit commandReceived(socket, socket->readAll());
}
