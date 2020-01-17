QT = core network
CONFIG += link_pkgconfig

#DEFINES += USE_DBUS
contains(DEFINES, USE_DBUS) {
    QT += dbus

    bridge_dbus_adaptor.files = ../dbus/ru.omprussia.qabridge.xml
    bridge_dbus_adaptor.source_flags = -c QABridgeAdaptor
    bridge_dbus_adaptor.header_flags = -c QABridgeAdaptor -i ../dbus/dbus_qabridge_include.h
    DBUS_ADAPTORS += bridge_dbus_adaptor

    dbusService.files = dbus/ru.omprussia.qabridge.service
    dbusService.path = /usr/share/dbus-1/system-services/
    INSTALLS += dbusService

    dbusConf.files = dbus/ru.omprussia.qabridge.conf
    dbusConf.path = /etc/dbus-1/system.d/
    INSTALLS += dbusConf

    SOURCES += \
        src/QAScreenRecorder.cpp

    HEADERS += \
        src/QAScreenRecorder.hpp
}

#DEFINES += USE_SYSTEMD
contains(DEFINES, USE_SYSTEMD) {
    PKGCONFIG += libsystemd-daemon

DEFINES += USE_PACKAGEKIT
PKGCONFIG += packagekitqt5

DEFINES += USE_RPM
PKGCONFIG += rpm

DEFINES += USE_CONNMAN
PKGCONFIG += connman-qt5

TEMPLATE = app
TARGET = qabridge
TARGETPATH = /usr/bin
target.path = $$TARGETPATH
INSTALLS += target

SOURCES += \
    src/main.cpp \
    src/QABridge.cpp

HEADERS += \
    src/QABridge.hpp
INCLUDEPATH += /usr/include
INCLUDEPATH += /usr/include/packagekitqt5/PackageKit
