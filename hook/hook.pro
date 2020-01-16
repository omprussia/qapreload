TEMPLATE = lib
QT =
CONFIG += plugin

SOURCES += \
    src/hook.cpp

TARGET = qapreloadhook
target.path = /usr/lib

INSTALLS = target
