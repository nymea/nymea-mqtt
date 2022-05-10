TEMPLATE = app
TARGET = nymea-mqtt-client

include(../nymea-mqtt.pri)

QT += network
QT -= gui

INCLUDEPATH += $$top_srcdir/libnymea-mqtt/

SOURCES += main.cpp

LIBS += -L$${top_builddir}/libnymea-mqtt -lnymea-mqtt

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target
