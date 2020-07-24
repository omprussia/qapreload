// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#ifndef QASERVICE_HPP
#define QASERVICE_HPP

#include <QObject>
#include <QVariant>

class IBridgePlatform;
class QABridgeSocketServer;
class QTcpSocket;
class QABridge : public QObject
{
    Q_OBJECT
public:
    explicit QABridge(QObject *parent = nullptr);
    static bool metaInvoke(QTcpSocket *socket, QObject *object, const QString &methodName, const QVariantList &params, bool *implemented = nullptr);

public slots:
    void start();

private slots:
    void removeClient(QTcpSocket *socket);

    void processCommand(QTcpSocket *socket, const QByteArray &cmd);
    void processAppConnectCommand(QTcpSocket *socket, const QJsonObject &app);
    bool processAppiumCommand(QTcpSocket *socket, const QString &action, const QVariantList &params);

private:
    IBridgePlatform *m_platform = nullptr;
    QABridgeSocketServer *m_socketServer = nullptr;
};

#endif // QASERVICE_HPP
