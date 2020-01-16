#include <unistd.h>
#include <pwd.h>

#include <dlfcn.h>

typedef void(*StartupCallback)();

static int qa_loaded = 0;
static int is_nemo = 0;

static StartupCallback next_startup_hook = nullptr;
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

extern "C" void qt_startup_hook()
{
    if (!is_nemo) {
        return;
    }
    if (qa_loaded) {
        return;
    }
    dlopen("libqaengine.so", RTLD_LAZY);
    qa_loaded = 1;

    static StartupCallback next_qt_startup_hook = reinterpret_cast<StartupCallback>(dlsym(RTLD_NEXT, "qt_startup_hook"));
    next_qt_startup_hook();

    if (next_startup_hook) {
        next_startup_hook();
    }
}

__attribute__((constructor))
static void libConstructor() {
    is_nemo = getuid() == nemo_uid();

    next_startup_hook = reinterpret_cast<StartupCallback>(qtHookData[StartupHookIndex]);
    qtHookData[StartupHookIndex] = reinterpret_cast<StartupCallback>(&qt_startup_hook);
}

