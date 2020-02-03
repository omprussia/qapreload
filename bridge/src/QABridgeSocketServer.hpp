#pragma once

#include <QObject>

class AppiumSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit AppiumSocketClient(QObject *parent = nullptr);

signals:

public slots:
};

