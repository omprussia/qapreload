# Copyright (c) 2019-2020 Open Mobile Platform LLC.
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
    TARGETPATH = $$[QT_INSTALL_LIBS]
    target.path = $$TARGETPATH
    INSTALLS = target
}
