#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#ifndef Q_OS_SAILFISH
#include <private/qhooks_p.h>
#else
#include <pwd.h>

typedef void(*StartupCallback)();
static const int StartupHookIndex = 5;
StartupCallback qtHookData[100];

inline uid_t nemo_uid()
{
    static struct passwd *nemo_pwd;

    if (!nemo_pwd) {
        nemo_pwd = getpwnam("nemo");
        if (!nemo_pwd) {
            return 100000;
        }
    }

    return nemo_pwd->pw_uid;
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
    if (getuid() != nemo_uid()) {
        return;
    }
    qtHookData[StartupHookIndex] = reinterpret_cast<StartupCallback>(&my_startup_hook);
#else
    qtHookData[QHooks::Startup] = reinterpret_cast<quintptr>(&my_startup_hook);
#endif

}
