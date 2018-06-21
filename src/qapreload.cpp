#include "QAHooks.hpp"

__attribute__((constructor))
static void libConstructor() {
    QAHooks::installHooks();
}

__attribute__((destructor))
static void libDestructor() {
    //
}
