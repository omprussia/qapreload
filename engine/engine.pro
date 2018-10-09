TEMPLATE = lib
QT = core dbus quick quick-private core-private
CONFIG += plugin
CONFIG += c++11

SOURCES += \
    src/engine.cpp \
    src/QAEngine.cpp \
    src/QAService.cpp \
    src/QAMouseEngine.cpp \
    src/QAKeyEngine.cpp \
    src/QAPendingEvent.cpp \
    src/SailfishTest.cpp \
    src/LipstickTestHelper.cpp \
    src/plugin.cpp

HEADERS += \
    src/QAEngine.hpp \
    src/QAService.hpp \
    src/QAMouseEngine.hpp \
    src/QAKeyEngine.hpp \
    src/QAPendingEvent.hpp \
    src/SailfishTest.hpp \
    src/LipstickTestHelper.hpp

TARGET = qaengine
target.path = /usr/lib/qtpreloadplugins

INSTALLS = target

qa_dbus_adaptor.files = dbus/ru.omprussia.qaservice.xml
qa_dbus_adaptor.source_flags = -c QAAdaptor
qa_dbus_adaptor.header_flags = -c QAAdaptor
DBUS_ADAPTORS += qa_dbus_adaptor

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


