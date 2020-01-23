TEMPLATE = lib
QT = core network quick quick-private core-private xmlpatterns
CONFIG += plugin
CONFIG += c++11
CONFIG += link_pkgconfig

contains(DEFINES, USE_MLITE5) {
    message("Building engine with mlite5 support")
    PKGCONFIG += mlite5
}

contains(DEFINES, USE_DBUS) {
    message("Building engine with dbus support")
    QT += dbus

    SOURCES += \
        src/QADBusService.cpp

    HEADERS += \
        src/QADBusService.hpp

    qa_dbus_adaptor.files = dbus/ru.omprussia.qaservice.xml
    qa_dbus_adaptor.source_flags = -c QAAdaptor
    qa_dbus_adaptor.header_flags = -c QAAdaptor
    DBUS_ADAPTORS += qa_dbus_adaptor
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


contains(DEFINES, USE_MER_QDOC) {
    message("Building engine with mer qdoc support")
    CONFIG += mer-qdoc-template
    MER_QDOC.project = qapreload
    MER_QDOC.config = doc/qapreload.qdocconf
    MER_QDOC.style = offline
    MER_QDOC.path = /usr/share/doc/qapreload

    OTHER_FILES += \
        doc/src/index.qdoc
}
