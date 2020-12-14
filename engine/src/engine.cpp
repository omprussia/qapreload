// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#include "QAEngine.hpp"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QTimer>

#ifndef Q_OS_SAILFISH
#include <private/qhooks_p.h>
#else
typedef void(*AddQObjectCallback)(QObject*);
typedef void(*RemoveQObjectCallback)(QObject*);
static const int AddQObjectHookIndex = 3;
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
    qtHookData[AddQObjectHookIndex] = reinterpret_cast<AddQObjectCallback>(&QAEngine::objectCreated);
#else
    qtHookData[QHooks::RemoveQObject] = reinterpret_cast<quintptr>(&QAEngine::objectRemoved);
    qtHookData[QHooks::AddQObject] = reinterpret_cast<quintptr>(&QAEngine::objectCreated);
#endif
    QTimer::singleShot(0, qApp, [](){ QAEngine::instance()->initialize(); });
}
