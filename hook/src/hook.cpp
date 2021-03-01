// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
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

#ifndef Q_OS_SAILFISH
#include <private/qhooks_p.h>
#else
#include <getdef.h>
#include <pwd.h>
#include <unistd.h>

typedef void(*StartupCallback)();
static const int StartupHookIndex = 5;
StartupCallback qtHookData[100];

inline uid_t user_uid()
{
    return getdef_num("UID_MIN", 100000);
}
#endif

static void my_startup_hook()
{
#ifdef Q_OS_WINDOWS
    LoadLibraryA("qaengine.dll");
#elif defined Q_OS_MACOS
    dlopen("libqaengine.dylib", RTLD_LAZY);
#else
    dlopen("libqaengine.so", RTLD_LAZY);
#endif
}

INITIALIZER(libConstructor)
{
#ifdef Q_OS_SAILFISH
    if (getuid() < user_uid()) {
        return;
    }
    qtHookData[StartupHookIndex] = reinterpret_cast<StartupCallback>(&my_startup_hook);
#else
    qtHookData[QHooks::Startup] = reinterpret_cast<quintptr>(&my_startup_hook);
#endif

}
