#include "QAHooks.hpp"

#include <unistd.h>
#include <pwd.h>

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

__attribute__((constructor))
static void libConstructor() {
    if (getuid() == nemo_uid()) {
        QAHooks::installStartupHook();
    }
}

__attribute__((destructor))
static void libDestructor() {
    //
}
