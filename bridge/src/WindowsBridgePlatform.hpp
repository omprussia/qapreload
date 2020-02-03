#pragma once
#include "GenericBridgePlatform.hpp"

class WindowsBridgePlatform : public GenericBridgePlatform
{
public:
    explicit WindowsBridgePlatform(QObject *parent);

protected:
    bool lauchAppPlatform(QTcpSocket *socket) override;
    bool lauchAppStandalone(const QString &appName, const QStringList &arguments = {}) override;
};

