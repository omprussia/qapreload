#pragma once
#include "LinuxBridgePlatform.hpp"


class MacBridgePlatform : public LinuxBridgePlatform
{
    Q_OBJECT
public:
    explicit MacBridgePlatform(QObject *parent);

protected:
    bool lauchAppStandalone(const QString &appName, const QStringList &arguments = {}) override;
};
