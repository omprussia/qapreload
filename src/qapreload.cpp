#include "QAHooks.hpp"

__attribute__((constructor))
static void libConstructor() {
    QAHooks::installAllHooks();
}

__attribute__((destructor))
static void libDestructor() {
    //
}
