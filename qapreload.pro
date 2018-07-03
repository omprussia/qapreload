TEMPLATE = lib
QT = core dbus quick quick-private core-private
CONFIG += plugin
CONFIG += c++11

SOURCES += \
    src/qapreload.cpp \
    src/QAHooks.cpp \
    src/QAEngine.cpp \
    src/QAService.cpp \
    src/QAMouseEngine.cpp

TARGET = qapreload
target.path = /usr/lib

INSTALLS = target

OTHER_FILES += rpm/qapreload.spec

HEADERS += \
    src/QAHooks.hpp \
    src/QAEngine.hpp \
    src/QAService.hpp \
    src/QAMouseEngine.hpp

qa_dbus_adaptor.files = dbus/ru.omprussia.qaservice.xml
qa_dbus_adaptor.source_flags = -c QAAdaptor
qa_dbus_adaptor.header_flags = -c QAAdaptor
DBUS_ADAPTORS += qa_dbus_adaptor
