#include "QAEngine.hpp"

__attribute__((constructor))
static void libConstructor() {
    QAEngine::initialize();
}
