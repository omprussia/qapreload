TEMPLATE = lib
QT =
CONFIG += plugin

SOURCES += \
    src/qapreload.cpp

TARGET = qapreload
target.path = /usr/lib

INSTALLS = target
