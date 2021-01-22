// Copyright (c) 2019-2020 Open Mobile Platform LLC.
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
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

__attribute__((constructor))
static void libConstructor() {
#ifdef Q_OS_SAILFISH
    if (getuid() < user_uid()) {
        return;
    }
    qtHookData[StartupHookIndex] = reinterpret_cast<StartupCallback>(&my_startup_hook);
#else
    qtHookData[QHooks::Startup] = reinterpret_cast<quintptr>(&my_startup_hook);
#endif

}
