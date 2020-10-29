// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAEngine.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QTimer>

#ifndef Q_OS_SAILFISH
#include <private/qhooks_p.h>
#else
typedef void(*RemoveQObjectCallback)(QObject*);
static const int RemoveQObjectHookIndex = 4;
RemoveQObjectCallback qtHookData[100];
#endif

__attribute__((constructor))
static void libConstructor() {
    QGuiApplication *gui = qobject_cast<QGuiApplication*>(qApp);
    if (!gui) {
        return;
    }

#ifdef Q_OS_SAILFISH
    qtHookData[RemoveQObjectHookIndex] = reinterpret_cast<RemoveQObjectCallback>(&QAEngine::objectRemoved);
#else
    qtHookData[QHooks::RemoveQObject ] = reinterpret_cast<quintptr>(&QAEngine::objectRemoved);
#endif
    QTimer::singleShot(0, qApp, [](){ QAEngine::instance()->initialize(); });
}
