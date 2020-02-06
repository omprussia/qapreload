TARGET = qapreloadhook
TEMPLATE = lib
QT =
CONFIG += plugin

SOURCES += \
    src/hook.cpp

! contains(DEFINES, Q_OS_SAILFISH) {
    QT += core core-private
}

unix {
    target.path = /usr/lib
    INSTALLS = target
}
