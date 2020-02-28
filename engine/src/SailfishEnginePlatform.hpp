#pragma once

#include "QuickEnginePlatform.hpp"

class SailfishEnginePlatform : public QuickEnginePlatform
{
    Q_OBJECT
public:
    explicit SailfishEnginePlatform(QObject *parent);

public slots:
    virtual void initialize() override;

private slots:
    // IEnginePlatform methods

    // execute_%1 methods

    // own methods
    void onChildrenChanged();
};

