TARGET = qapreloadhook
TEMPLATE = lib
QT += core
CONFIG += plugin

SOURCES += \
    src/hook.cpp

unix {
    target.path = /usr/lib
    INSTALLS = target
}
