#include <unistd.h>
#include <pwd.h>

#include <dlfcn.h>

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

extern "C" void qt_startup_hook()
{
    if (getuid() != nemo_uid()) {
        return;
    }

    dlopen("libqaengine.so", RTLD_LAZY);
}
