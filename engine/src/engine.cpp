#include "QAPreloadEngine.hpp"

__attribute__((constructor))
static void libConstructor() {
    QAPreloadEngine::initialize();
}
