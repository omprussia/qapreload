TEMPLATE = subdirs
SUBDIRS = \
    engine \
    bridge

dbusInterface.files = dbus/ru.omprussia.qabridge.xml
dbusInterface.path = /usr/share/dbus-1/interfaces/
INSTALLS += dbusInterface

OTHER_FILES += \
    rpm/qapreload.spec \
    dbus/dbus_qabridge_include.h
