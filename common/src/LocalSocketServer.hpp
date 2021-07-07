// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once

#include "ITransportServer.hpp"
#include <QHash>

class QLocalServer;
class LocalSocketServer : public ITransportServer
{
    Q_OBJECT
public:
    explicit LocalSocketServer(const QString &name, QObject *parent = nullptr);

public slots:
    void start() override;

private slots:
    void newConnection();

private:
    QString m_name;
    QLocalServer *m_server = nullptr;
};

