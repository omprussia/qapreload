TEMPLATE = lib
QT =
CONFIG += plugin

SOURCES += \
    src/preload.cpp

TARGET = qapreload
target.path = /usr/lib

INSTALLS = target
