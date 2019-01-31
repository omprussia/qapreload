QT = core dbus network
CONFIG += link_pkgconfig
PKGCONFIG += \
    libsystemd-daemon \
    libshadowutils \
    packagekitqt5 \
    connman-qt5

TEMPLATE = app
TARGET = qabridge
TARGETPATH = /usr/bin
target.path = $$TARGETPATH
INSTALLS += target

autostart.files = \
    systemd/qabridge.service \
    systemd/qabridge.socket
autostart.path = /lib/systemd/system
INSTALLS += autostart

dbusService.files = dbus/ru.omprussia.qabridge.service
dbusService.path = /usr/share/dbus-1/system-services/
INSTALLS += dbusService

dbusConf.files = dbus/ru.omprussia.qabridge.conf
dbusConf.path = /etc/dbus-1/system.d/
INSTALLS += dbusConf

SOURCES += \
    src/main.cpp \
    src/QABridge.cpp

HEADERS += \
    src/QABridge.hpp

bridge_dbus_adaptor.files = ../dbus/ru.omprussia.qabridge.xml
bridge_dbus_adaptor.source_flags = -c QABridgeAdaptor
bridge_dbus_adaptor.header_flags = -c QABridgeAdaptor -i ../dbus/dbus_qabridge_include.h
DBUS_ADAPTORS += bridge_dbus_adaptor

INCLUDEPATH += /usr/include
INCLUDEPATH += /usr/include/packagekitqt5/PackageKit
