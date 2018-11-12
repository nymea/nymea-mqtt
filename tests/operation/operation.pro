QT += testlib network
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

include(../../nymea-mqtt.pri)


INCLUDEPATH += $$top_srcdir/libnymea-mqtt/

SOURCES += test_operation.cpp

LIBS += -L$$top_builddir/libnymea-mqtt/ -lnymea-mqtt
