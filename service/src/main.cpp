// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "qauserservice.h"

#include <QCoreApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QAUserService service;
    QTimer::singleShot(0, qApp, [&service](){ service.start(); });
    return app.exec();
}
