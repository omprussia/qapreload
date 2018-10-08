QT = core dbus network
CONFIG += link_pkgconfig
PKGCONFIG += \
    libsystemd-daemon \
    libshadowutils

TEMPLATE = app
TARGET = qabridge
TARGETPATH = /usr/bin
target.path = $$TARGETPATH

autostart.files = \
    systemd/qabridge.service \
    systemd/qabridge.socket
autostart.path = /lib/systemd/system

SOURCES += \
    src/main.cpp \
    src/QABridge.cpp

HEADERS += \
    src/QABridge.hpp

INSTALLS += target autostart
