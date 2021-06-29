# Copyright (c) 2019-2020 Open Mobile Platform LLC.
TEMPLATE = lib
QT = core network quick quick-private core-private xmlpatterns
CONFIG += plugin
CONFIG += c++11
CONFIG += link_pkgconfig

isEmpty(PROJECT_PACKAGE_VERSION) {
    QAPRELOAD_VERSION = "2.0.0-dev"
} else {
    QAPRELOAD_VERSION = $$PROJECT_PACKAGE_VERSION
}

message("QAPRELOAD_VERSION: $$QAPRELOAD_VERSION")
DEFINES += QAPRELOAD_VERSION=\\\"$$QAPRELOAD_VERSION\\\"

contains(DEFINES, Q_OS_SAILFISH) {
    message("Building for sailfish os")

    SOURCES += \
        ../../common/src/LocalSocketClient.cpp \
        src/SailfishEnginePlatform.cpp

    HEADERS += \
        ../../common/src/LocalSocketClient.hpp \
        src/SailfishEnginePlatform.hpp

} else {
    message("Building engine with widgets support")
    QT += widgets widgets-private

    SOURCES += \
        src/WidgetsEnginePlatform.cpp

    HEADERS += \
        src/WidgetsEnginePlatform.hpp

}

contains(DEFINES, USE_MLITE5) {
    message("Building engine with mlite5 support")
    PKGCONFIG += mlite5
}

contains(DEFINES, USE_DBUS) {
    message("Building engine with dbus support")
    QT += dbus

    qa_dbus_adaptor.files = dbus/ru.omprussia.qaservice.xml
    qa_dbus_adaptor.source_flags = -c QAAdaptor
    qa_dbus_adaptor.header_flags = -c QAAdaptor
    DBUS_ADAPTORS += qa_dbus_adaptor
}

INCLUDEPATH += ../../common/src

SOURCES += \
    src/engine.cpp \
    src/QAEngine.cpp \
    ../../common/src/TCPSocketClient.cpp \
    src/QAEngineSocketClient.cpp \
    src/GenericEnginePlatform.cpp \
    src/QuickEnginePlatform.cpp \
    src/QAMouseEngine.cpp \
    src/QAKeyEngine.cpp \
    src/QAPendingEvent.cpp

HEADERS += \
    src/QAEngine.hpp \
    ../../common/src/ITransportClient.hpp \
    ../../common/src/TCPSocketClient.hpp \
    src/QAEngineSocketClient.hpp \
    src/IEnginePlatform.hpp \
    src/GenericEnginePlatform.hpp \
    src/QuickEnginePlatform.hpp \
    src/QAMouseEngine.hpp \
    src/QAKeyEngine.hpp \
    src/QAPendingEvent.hpp

TARGET = qaengine
TARGETPATH = $$[QT_INSTALL_LIBS]
target.path = $$TARGETPATH

INSTALLS = target
