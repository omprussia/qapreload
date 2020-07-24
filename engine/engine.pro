# Copyright (c) 2019-2020 Open Mobile Platform LLC.
TEMPLATE = lib
QT = core network quick quick-private core-private xmlpatterns
CONFIG += plugin
CONFIG += c++11
CONFIG += link_pkgconfig

contains(DEFINES, Q_OS_SAILFISH) {
    message("Building for sailfish os")

    SOURCES += \
        src/SailfishEnginePlatform.cpp

    HEADERS += \
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

SOURCES += \
    src/engine.cpp \
    src/QAEngine.cpp \
    src/QAEngineSocketClient.cpp \
    src/GenericEnginePlatform.cpp \
    src/QuickEnginePlatform.cpp \
    src/QAMouseEngine.cpp \
    src/QAKeyEngine.cpp \
    src/QAPendingEvent.cpp

HEADERS += \
    src/QAEngine.hpp \
    src/QAEngineSocketClient.hpp \
    src/IEnginePlatform.hpp \
    src/GenericEnginePlatform.hpp \
    src/QuickEnginePlatform.hpp \
    src/QAMouseEngine.hpp \
    src/QAKeyEngine.hpp \
    src/QAPendingEvent.hpp

TARGET = qaengine
target.path = /usr/lib

INSTALLS = target
