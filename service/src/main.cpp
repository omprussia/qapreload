#include "qauserservice.h"

#include <QCoreApplication>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QAUserService service;
    QTimer::singleShot(0, &service, &QAUserService::start);
    return app.exec();
}
