QT -= gui
QT += network

CONFIG += c++11 console static
CONFIG -= app_bundle

SOURCES += \
    mqttserver.cpp \
    mqttpacket.cpp \
    mqttsubscription.cpp \
    $$PWD/mqttclient.cpp

HEADERS += \
    mqttserver.h \
    mqttpacket.h \
    mqtt.h \
    mqttsubscription.h \
    $$PWD/mqttclient.h \
    $$PWD/mqttpacket_p.h \
    $$PWD/mqttclient_p.h \
    $$PWD/mqttserver_p.h

