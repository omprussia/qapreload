#ifdef WIN32
#include <windows.h>
#include <iostream>
#include <stdio.h>
#else
#include <pwd.h>
#include <dlfcn.h>
#endif

#include <QCoreApplication>

typedef void(*StartupCallback)();

static int qa_loaded = 0;
static int is_user = 0;

static StartupCallback next_startup_hook = nullptr;
static const int StartupHookIndex = 5;

StartupCallback qtHookData[100];

#ifdef Q_OS_SAILFISH
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

void loadEngine()
{
#ifdef Q_OS_WINDOWS
    LoadLibraryA("qaengine.dll");
#else
    dlopen("libqaengine.so", RTLD_LAZY);
#endif

    qa_loaded = 1;
}

extern "C" void qt_startup_hook()
{
    if (!is_user) {
        return;
    }
    if (qa_loaded) {
        return;
    }

    loadEngine();
}

__attribute__((constructor))
static void libConstructor() {
#ifdef Q_OS_SAILFISH
    is_user = getuid() == nemo_uid();
#else
    is_user = true;
#endif

    if (qApp) {
        loadEngine();
    } else {
        next_startup_hook = reinterpret_cast<StartupCallback>(qtHookData[StartupHookIndex]);
        qtHookData[StartupHookIndex] = reinterpret_cast<StartupCallback>(&qt_startup_hook);
    }

}
