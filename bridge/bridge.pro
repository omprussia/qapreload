QT = core network
CONFIG += link_pkgconfig

contains(DEFINES, USE_DBUS) {
    message("Building bridge with dbus support")

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

contains(DEFINES, USE_SYSTEMD) {
    message("Building bridge with systemd support")
    PKGCONFIG += libsystemd-daemon

    autostart.files = \
        systemd/qabridge.service \
        systemd/qabridge.socket
    autostart.path = /lib/systemd/system
    INSTALLS += autostart
}

contains(DEFINES, USE_PACKAGEKIT) {
    message("Building bridge with packagekit support")
    PKGCONFIG += packagekitqt5

    INCLUDEPATH += /usr/include/packagekitqt5/PackageKit
}

contains(DEFINES, USE_RPM) {
    message("Building bridge with rpm support")
    PKGCONFIG += rpm
}

contains(DEFINES, USE_CONNMAN) {
    message("Building bridge with connman support")
    PKGCONFIG += connman-qt5
}

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
