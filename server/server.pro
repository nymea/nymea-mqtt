TEMPLATE = app
TARGET = nymea-mqttserver

include(../nymea-mqtt.pri)

QT += network

INCLUDEPATH += $$top_srcdir/libnymea-mqtt/

SOURCES += main.cpp

LIBS += -L$$top_builddir/libnymea-mqtt/ -lnymea-mqtt

target.path = /usr/bin/
INSTALLS += target
