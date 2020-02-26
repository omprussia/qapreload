#pragma once

#include "GenericEnginePlatform.hpp"

class WidgetsEnginePlatform : public GenericEnginePlatform
{
    Q_OBJECT
public:
    explicit WidgetsEnginePlatform(QObject *parent);

public slots:
    virtual void initialize() override;
};

