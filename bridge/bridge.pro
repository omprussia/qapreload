QT = core network
CONFIG += link_pkgconfig

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

SOURCES += \
    src/main.cpp \
    src/QABridge.cpp

HEADERS += \
    src/QABridge.hpp

win32 {
    HEADERS += \
        src/WinInjector.hpp

    SOURCES += \
        src/WinInjector.cpp
}

contains(DEFINES, Q_OS_SAILFISH) {
    PKGCONFIG += libshadowutils

    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/libshadowutils

    TARGETPATH = /usr/bin
    target.path = $$TARGETPATH
    INSTALLS += target
}
