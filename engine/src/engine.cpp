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

#ifdef __cplusplus
    #define INITIALIZER(f) \
        static void f(void); \
        struct f##_t_ { f##_t_(void) { f(); } }; static f##_t_ f##_; \
        static void f(void)
#elif defined(_MSC_VER)
    #pragma section(".CRT$XCU",read)
    #define INITIALIZER2_(f,p) \
        static void f(void); \
        __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f; \
        __pragma(comment(linker,"/include:" p #f "_")) \
        static void f(void)
    #ifdef _WIN64
        #define INITIALIZER(f) INITIALIZER2_(f,"")
    #else
        #define INITIALIZER(f) INITIALIZER2_(f,"_")
    #endif
#else
    #define INITIALIZER(f) \
        static void f(void) __attribute__((constructor)); \
        static void f(void)
#endif

INITIALIZER(libConstructor){
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
