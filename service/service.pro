# Copyright (c) 2019-2020 Open Mobile Platform LLC.
QT = core dbus
CONFIG += link_pkgconfig
PKGCONFIG += \
    contentaction5

isEmpty(PROJECT_PACKAGE_VERSION) {
    QAPRELOAD_VERSION = "2.0.0-dev"
} else {
    QAPRELOAD_VERSION = $$PROJECT_PACKAGE_VERSION
}

message("QAPRELOAD_VERSION: $$QAPRELOAD_VERSION")
DEFINES += QAPRELOAD_VERSION=\\\"$$QAPRELOAD_VERSION\\\"

message("Building user service")

TEMPLATE = app
TARGET = qabridge-user
TARGETPATH = /usr/bin
target.path = $$TARGETPATH
INSTALLS += target

autostart.files = systemd/qaservice.service
autostart.path = $$[QT_INSTALL_LIBS]/systemd/user
INSTALLS += autostart

dbus.files = dbus/ru.omprussia.qaservice.service
dbus.path = /usr/share/dbus-1/services/
INSTALLS += dbus

SOURCES += \
    src/main.cpp \
    src/qauserservice.cpp

HEADERS += \
    src/qauserservice.h

