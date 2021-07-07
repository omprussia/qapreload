// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "ITransportClient.hpp"
#include "ITransportServer.hpp"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>

void ITransportServer::registerClient(ITransportClient *client)
{
    connect(client, &ITransportClient::readyRead, this, &ITransportServer::readData);
    connect(client, &ITransportClient::disconnected, this, &ITransportServer::clientLost);
}

void ITransportServer::readData(ITransportClient *client)
{
    auto bytes = client->bytesAvailable();
    qDebug()
        << Q_FUNC_INFO
        << client << bytes;

    static QByteArray requestData;
    requestData.append(client->readAll());
    qDebug().noquote() << requestData;

    requestData.replace("}{", "}\n{"); // workaround packets join

    const QList<QByteArray> commands = requestData.split('\n');
    qDebug()
        << Q_FUNC_INFO
        << "Commands count:" << commands.length();

    requestData.clear();
    for (const QByteArray &cmd : commands) {
        if (cmd.isEmpty()) {
            continue;
        }
        qDebug()
            << Q_FUNC_INFO
            << "Command:";
        qDebug().noquote() << cmd;

        QJsonParseError error;
        QJsonDocument::fromJson(cmd, &error);
        if (error.error != QJsonParseError::NoError) {
            qDebug()
                << Q_FUNC_INFO
                << "Partial data, waiting for more";
            qDebug().noquote() << error.errorString();

            requestData = cmd;
            break;
        }

        emit commandReceived(client, cmd);
    }

}
