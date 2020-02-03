#pragma once
#include "GenericBridgePlatform.hpp"

class LinuxBridgePlatform : public GenericBridgePlatform
{
    Q_OBJECT
public:
    explicit LinuxBridgePlatform(QObject *parent);

protected:
    virtual bool lauchAppPlatform(QTcpSocket *socket) override;
    virtual bool lauchAppStandalone(const QString &appName, const QStringList &arguments = {}) override;
};

