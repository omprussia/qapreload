// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once
#include "GenericBridgePlatform.hpp"

class LinuxBridgePlatform : public GenericBridgePlatform
{
    Q_OBJECT
public:
    explicit LinuxBridgePlatform(QObject *parent);

protected:
    virtual bool lauchAppPlatform(ITransportClient *socket) override;
    virtual bool lauchAppStandalone(const QString &appName, const QStringList &arguments = {}) override;
};

