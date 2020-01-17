TEMPLATE = lib
QT = core network quick quick-private core-private xmlpatterns
CONFIG += plugin
CONFIG += c++11
CONFIG += link_pkgconfig
PKGCONFIG += mlite5

#DEFINES += USE_DBUS
contains(DEFINES, USE_DBUS) {
    QT += dbus

    SOURCES += \
        src/QADBusService.cpp

    HEADERS += \
        src/QADBusService.hpp

    qa_dbus_adaptor.files = dbus/ru.omprussia.qaservice.xml
    qa_dbus_adaptor.source_flags = -c QAAdaptor
    qa_dbus_adaptor.header_flags = -c QAAdaptor
    DBUS_ADAPTORS += qa_dbus_adaptor

    bridge_dbus_interface.files = ../dbus/ru.omprussia.qabridge.xml
    bridge_dbus_interface.source_flags = -c QABridgeInterface
    bridge_dbus_interface.header_flags = -c QABridgeInterface -i ../dbus/dbus_qabridge_include.h
    DBUS_INTERFACES += bridge_dbus_interface
}

SOURCES += \
    src/engine.cpp \
    src/QAPreloadEngine.cpp \
    src/QAEngine.cpp \
    src/QAMouseEngine.cpp \
    src/QAKeyEngine.cpp \
    src/QAPendingEvent.cpp \
    src/SailfishTest.cpp \
    src/LipstickTestHelper.cpp \
    src/plugin.cpp \
    src/QASocketService.cpp

HEADERS += \
    src/QAPreloadEngine.hpp \
    src/QAEngine.hpp \
    src/QAMouseEngine.hpp \
    src/QAKeyEngine.hpp \
    src/QAPendingEvent.hpp \
    src/SailfishTest.hpp \
    src/LipstickTestHelper.hpp \
    src/QASocketService.hpp

TARGET = qaengine
target.path = /usr/lib

INSTALLS = target

qmlfiles.files = \
    qml/TouchIndicator.qml
qmlfiles.path = /usr/share/qapreload/qml
INSTALLS += qmlfiles

qml.files = qmldir
qml.path = /usr/lib/qt5/qml/ru/omprussia/sailfishtest
INSTALLS += qml

CONFIG += mer-qdoc-template
MER_QDOC.project = qapreload
MER_QDOC.config = doc/qapreload.qdocconf
MER_QDOC.style = offline
MER_QDOC.path = /usr/share/doc/qapreload

OTHER_FILES += \
    doc/src/index.qdoc


