QT = core dbus
CONFIG += link_pkgconfig
PKGCONFIG += \
    contentaction5

TEMPLATE = app
TARGET = qabridge-user
TARGETPATH = /usr/bin
target.path = $$TARGETPATH
INSTALLS += target

autostart.files = systemd/qaservice.service
autostart.path = /usr/lib/systemd/user
INSTALLS += autostart

dbus.files = dbus/ru.omprussia.qaservice.service
dbus.path = /usr/share/dbus-1/services/
INSTALLS += dbus

SOURCES += \
    src/main.cpp \
    src/qauserservice.cpp

HEADERS += \
    src/qauserservice.h
