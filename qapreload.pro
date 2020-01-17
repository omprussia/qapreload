TEMPLATE = subdirs
SUBDIRS = \
    hook \
    engine \
    bridge \
    service

#DEFINES += USE_DBUS
contains(DEFINES, USE_DBUS) {
    dbusInterface.files = dbus/ru.omprussia.qabridge.xml
    dbusInterface.path = /usr/share/dbus-1/interfaces/
    INSTALLS += dbusInterface

    OTHER_FILES += dbus/dbus_qabridge_include.h
}

OTHER_FILES += \
    rpm/qapreload.spec
