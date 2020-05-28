# Copyright (c) 2020 Open Mobile Platform LLС.
TARGET = qapreloadhook
TEMPLATE = lib

contains(DEFINES, Q_OS_SAILFISH) {
    QT =
} else {
    QT = core core-private
}

CONFIG += plugin

SOURCES += \
    src/hook.cpp

unix {
    target.path = /usr/lib
    INSTALLS = target
}
