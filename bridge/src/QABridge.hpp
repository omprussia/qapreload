// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#ifndef QASERVICE_HPP
#define QASERVICE_HPP

#include <QObject>
#include <QVariant>

class IBridgePlatform;
class ITransportServer;
class ITransportClient;
class QABridge : public QObject
{
    Q_OBJECT
public:
    explicit QABridge(QObject *parent = nullptr);
    static bool metaInvoke(ITransportClient *client, QObject *object, const QString &methodName, const QVariantList &params, bool *implemented = nullptr);

public slots:
    void start();

    void removeClient(ITransportClient *client);

    void processCommand(ITransportClient *client, const QByteArray &cmd);
    void processAppConnectCommand(ITransportClient *client, const QJsonObject &app);
    bool processAppiumCommand(ITransportClient *client, const QString &action, const QVariantList &params);

private:
    IBridgePlatform *m_platform = nullptr;
    ITransportServer *m_socketServer = nullptr;
};

#endif // QASERVICE_HPP
