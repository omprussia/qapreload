#include <unistd.h>
#include <pwd.h>

#include <dlfcn.h>

unsigned int qtHookData[7];

static uid_t nemo_uid()
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

static void qt_startup_hook()
{
    dlopen("libqaengine.so", RTLD_LAZY);
}

__attribute__((constructor))
static void libConstructor() {
    if (getuid() != nemo_uid()) {
        return;
    }
    qtHookData[5] = reinterpret_cast<unsigned int>(&qt_startup_hook);
}

