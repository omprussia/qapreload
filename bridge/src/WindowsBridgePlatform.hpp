// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#pragma once
#include "GenericBridgePlatform.hpp"

class WindowsBridgePlatform : public GenericBridgePlatform
{
public:
    explicit WindowsBridgePlatform(QObject *parent);

protected:
    bool lauchAppPlatform(ITransportClient *socket) override;
    bool lauchAppStandalone(const QString &appName, const QStringList &arguments = {}) override;
};

