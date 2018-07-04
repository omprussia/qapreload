TEMPLATE = lib
QT = core dbus quick quick-private core-private
CONFIG += plugin
CONFIG += c++11

SOURCES += \
    src/engine.cpp \
    src/QAEngine.cpp \
    src/QAService.cpp \
    src/QAMouseEngine.cpp

TARGET = qaengine
target.path = /usr/lib

INSTALLS = target

HEADERS += \
    src/QAEngine.hpp \
    src/QAService.hpp \
    src/QAMouseEngine.hpp

qa_dbus_adaptor.files = dbus/ru.omprussia.qaservice.xml
qa_dbus_adaptor.source_flags = -c QAAdaptor
qa_dbus_adaptor.header_flags = -c QAAdaptor
DBUS_ADAPTORS += qa_dbus_adaptor
