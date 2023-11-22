QT += testlib network websockets
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

include(../../nymea-mqtt.pri)

INCLUDEPATH += $$top_srcdir/libnymea-mqtt/

HEADERS += $${top_srcdir}/tests/common/mqtttests.h

SOURCES += $${top_srcdir}/tests/common/mqtttests.cpp

LIBS += -L$$top_builddir/libnymea-mqtt/ -lnymea-mqtt
