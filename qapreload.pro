# Copyright (c) 2019-2020 Open Mobile Platform LLC.
TEMPLATE = subdirs

SUBDIRS = \
    hook \
    engine \
    bridge

contains(DEFINES, Q_OS_SAILFISH) {
    message("Building for SFOS")
    SUBDIRS += service

    OTHER_FILES += \
        rpm/qapreload.spec \
        dbus/dbus_qabridge_include.h
}
