// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAEngine.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QTimer>

__attribute__((constructor))
static void libConstructor() {
    QGuiApplication *gui = qobject_cast<QGuiApplication*>(qApp);
    if (!gui) {
        return;
    }
    QTimer::singleShot(0, qApp, [](){ QAEngine::instance()->initialize(); });
}
