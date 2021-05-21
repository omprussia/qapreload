# Copyright (c) 2019-2020 Open Mobile Platform LLC.
QT = core network
CONFIG += link_pkgconfig

isEmpty(PROJECT_PACKAGE_VERSION) {
    QAPRELOAD_VERSION = "2.0.0-dev"
} else {
    QAPRELOAD_VERSION = $$PROJECT_PACKAGE_VERSION
}

message("QAPRELOAD_VERSION: $$QAPRELOAD_VERSION")
DEFINES += QAPRELOAD_VERSION=\\\"$$QAPRELOAD_VERSION\\\"

contains(DEFINES, USE_DBUS) {
    message("Building bridge with dbus support")

    QT += dbus

    SOURCES += \
        src/QAScreenRecorder.cpp

    HEADERS += \
        src/QAScreenRecorder.hpp
}

contains(DEFINES, USE_SYSTEMD) {
    message("Building bridge with systemd support")
    PKGCONFIG += libsystemd-daemon

    message("SPEC_UNITDIR: $$SPEC_UNITDIR")

    autostart.files = \
        systemd/qabridge.service \
        systemd/qabridge.socket
    autostart.path = $$SPEC_UNITDIR
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

SOURCES += \
    src/GenericBridgePlatform.cpp \
    src/QABridgeSocketServer.cpp \
    src/main.cpp \
    src/QABridge.cpp

HEADERS += \
    src/GenericBridgePlatform.hpp \
    src/IBridgePlatform.hpp \
    src/QABridge.hpp \
    src/QABridgeSocketServer.hpp

win32 {
    CONFIG += console
    HEADERS += \
        src/WinInjector.hpp \
        src/WindowsBridgePlatform.hpp

    SOURCES += \
        src/WinInjector.cpp \
        src/WindowsBridgePlatform.cpp
}

linux {
    SOURCES += \
        src/LinuxBridgePlatform.cpp \

    HEADERS += \
        src/LinuxBridgePlatform.hpp
}

macx {
    SOURCES += \
        src/MacBridgePlatform.cpp

    HEADERS += \
        src/MacBridgePlatform.hpp
}

contains(DEFINES, Q_OS_SAILFISH) {
    PKGCONFIG += libshadowutils

    INCLUDEPATH += /usr/include/libshadowutils
    INCLUDEPATH += /usr/include/packagekitqt5/PackageKit

    SOURCES += \
        src/SailfishBridgePlatform.cpp \
        src/sailfishinjector/injector.c \
        src/sailfishinjector/elf.c \
        src/sailfishinjector/ptrace.c \
        src/sailfishinjector/remote_call.c \
        src/sailfishinjector/util.c

    HEADERS += \
        src/SailfishBridgePlatform.hpp \
        src/sailfishinjector/injector_internal.h \
        src/sailfishinjector/injector.h

    TARGETPATH = /usr/bin
    target.path = $$TARGETPATH
    INSTALLS += target
}
